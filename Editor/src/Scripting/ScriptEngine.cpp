#include "H_ScriptEngine.hpp"

#include <Maiden/Asset/H_AssetSystem.h>
#include <Maiden/Project/H_Project.h>
#include <Maiden/Scene/Components.hpp>
#include <Maiden/Scene/H_Scene.hpp>

#include <array>
#include <cstdio>
#include <fstream>
#include <tuple>
#include <vector>

namespace Madam::Scripting
{
	namespace
	{
		using HostInitialize = int32_t(__cdecl*)(const wchar_t*, const void*, void**);
		using HostGetDelegate = int32_t(__cdecl*)(void*, int, void**);
		using HostClose = int32_t(__cdecl*)(void*);
		using LoadAssembly = int(__cdecl*)(const wchar_t*, const wchar_t*, const wchar_t*,
			const wchar_t*, void*, void**);

		std::filesystem::path ExecutableDirectory()
		{
			std::array<wchar_t, 32768> buffer{};
			const DWORD count = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
			return count > 0 && count < buffer.size()
				? std::filesystem::path(buffer.data()).parent_path() : std::filesystem::path{};
		}

		std::filesystem::path DotnetRoot()
		{
			std::array<wchar_t, 32768> buffer{};
			const DWORD count = GetEnvironmentVariableW(L"DOTNET_ROOT", buffer.data(),
				static_cast<DWORD>(buffer.size()));
			if (count > 0 && count < buffer.size() &&
				std::filesystem::is_regular_file(std::filesystem::path(buffer.data()) / "dotnet.exe"))
				return buffer.data();
			const DWORD found = SearchPathW(nullptr, L"dotnet.exe", nullptr,
				static_cast<DWORD>(buffer.size()), buffer.data(), nullptr);
			if (found > 0 && found < buffer.size())
				return std::filesystem::path(buffer.data()).parent_path();
			const auto defaultRoot = std::filesystem::path(L"C:\\Program Files\\dotnet");
			return std::filesystem::is_regular_file(defaultRoot / "dotnet.exe") ? defaultRoot : std::filesystem::path{};
		}

		std::filesystem::path HostFxrPath()
		{
			const auto root = DotnetRoot() / "host" / "fxr";
			std::tuple<int, int, int> latest{};
			std::filesystem::path path;
			std::error_code error;
			std::filesystem::directory_iterator it(root, error), end;
			for (; it != end && !error; it.increment(error))
			{
				if (!it->is_directory(error)) continue;
				int major = 0, minor = 0, patch = 0;
				if (std::swscanf(it->path().filename().c_str(), L"%d.%d.%d", &major, &minor, &patch) != 3)
					continue;
				const auto candidate = it->path() / "hostfxr.dll";
				if (std::filesystem::is_regular_file(candidate) &&
					std::make_tuple(major, minor, patch) > latest)
				{
					latest = { major, minor, patch };
					path = candidate;
				}
			}
			return path;
		}

		bool RunBuild(const std::filesystem::path& dotnet, const std::filesystem::path& project,
			const std::filesystem::path& directory, bool skipRestore, std::string& output)
		{
			SECURITY_ATTRIBUTES security{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
			HANDLE readPipe = nullptr, writePipe = nullptr;
			if (!CreatePipe(&readPipe, &writePipe, &security, 0)) return false;
			SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);
			STARTUPINFOW startup{};
			startup.cb = sizeof(startup);
			startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
			startup.wShowWindow = SW_HIDE;
			startup.hStdOutput = writePipe;
			startup.hStdError = writePipe;
			startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
			PROCESS_INFORMATION process{};
			std::wstring command = L"\"" + dotnet.wstring() + L"\" build \"" + project.wstring() +
				L"\" --configuration Debug --nologo --verbosity quiet -p:RestoreIgnoreFailedSources=true";
			if (skipRestore) command += L" --no-restore";
			std::vector<wchar_t> arguments(command.begin(), command.end());
			arguments.push_back(L'\0');
			const BOOL started = CreateProcessW(dotnet.c_str(), arguments.data(), nullptr, nullptr,
				TRUE, CREATE_NO_WINDOW, nullptr, directory.c_str(), &startup, &process);
			CloseHandle(writePipe);
			if (!started)
			{
				CloseHandle(readPipe);
				output = "Could not start dotnet build (Win32 " + std::to_string(GetLastError()) + ").";
				return false;
			}
			std::array<char, 4096> chunk{};
			DWORD count = 0;
			while (ReadFile(readPipe, chunk.data(), static_cast<DWORD>(chunk.size()), &count, nullptr) && count)
				if (output.size() < 131072) output.append(chunk.data(), count);
			CloseHandle(readPipe);
			WaitForSingleObject(process.hProcess, INFINITE);
			DWORD exitCode = 1;
			GetExitCodeProcess(process.hProcess, &exitCode);
			CloseHandle(process.hThread);
			CloseHandle(process.hProcess);
			return exitCode == 0;
		}
	}

	ScriptEngine* ScriptEngine::activeInstance = nullptr;

	ScriptEngine::~ScriptEngine()
	{
		Stop();
		if (hostfxr) FreeLibrary(hostfxr);
	}

	bool ScriptEngine::BuildScripts(const std::filesystem::path& projectDirectory,
		std::filesystem::path& assemblyPath)
	{
		const auto managedApi = ExecutableDirectory() / "Managed" / "Maiden.Managed.dll";
		const auto dotnet = DotnetRoot() / "dotnet.exe";
		if (!std::filesystem::is_regular_file(managedApi) || !std::filesystem::is_regular_file(dotnet))
		{
			MADAM_ERROR("C# scripting requires the editor Managed files and a .NET 8 SDK.");
			return false;
		}
		const auto scriptDirectory = projectDirectory / "Library" / "Scripts";
		std::error_code error;
		std::filesystem::create_directories(scriptDirectory, error);
		if (error) { MADAM_ERROR("Could not create script library: {0}", error.message()); return false; }
		const auto copiedApi = scriptDirectory / "Maiden.Managed.dll";
		bool apiChanged = !std::filesystem::is_regular_file(copiedApi) ||
			std::filesystem::file_size(copiedApi) != std::filesystem::file_size(managedApi);
		if (!apiChanged)
		{
			std::ifstream source(managedApi, std::ios::binary), destination(copiedApi, std::ios::binary);
			apiChanged = !std::equal(std::istreambuf_iterator<char>(source), std::istreambuf_iterator<char>(),
				std::istreambuf_iterator<char>(destination));
		}
		if (apiChanged)
		{
			std::filesystem::copy_file(managedApi, copiedApi,
				std::filesystem::copy_options::overwrite_existing, error);
			if (error) { MADAM_ERROR("Could not copy managed API: {0}", error.message()); return false; }
		}

		const auto projectFile = scriptDirectory / "GameScripts.csproj";
		const std::string contents = R"(<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net8.0</TargetFramework>
    <AssemblyName>GameScripts</AssemblyName>
    <EnableDefaultCompileItems>false</EnableDefaultCompileItems>
    <Nullable>enable</Nullable>
    <ImplicitUsings>enable</ImplicitUsings>
  </PropertyGroup>
  <ItemGroup>
    <Compile Include="../../Assets/**/*.cs" />
    <Reference Include="Maiden.Managed"><HintPath>Maiden.Managed.dll</HintPath></Reference>
  </ItemGroup>
</Project>
)";
		std::ifstream previous(projectFile);
		const std::string existing((std::istreambuf_iterator<char>(previous)), std::istreambuf_iterator<char>());
		const bool projectChanged = existing != contents;
		if (projectChanged)
		{
			std::ofstream file(projectFile, std::ios::trunc);
			if (!file) { MADAM_ERROR("Could not create C# project: {0}", projectFile.string()); return false; }
			file << contents;
			if (!file.good()) return false;
		}
		std::string output;
		const bool skipRestore = !projectChanged &&
			std::filesystem::is_regular_file(scriptDirectory / "obj" / "project.assets.json");
		if (!RunBuild(dotnet, projectFile, scriptDirectory, skipRestore, output))
		{
			MADAM_ERROR("C# build failed:\n{0}", output);
			return false;
		}
		assemblyPath = scriptDirectory / "bin" / "Debug" / "net8.0" / "GameScripts.dll";
		if (!std::filesystem::is_regular_file(assemblyPath))
		{
			MADAM_ERROR("C# build did not produce {0}", assemblyPath.string());
			return false;
		}
		MADAM_INFO("C# scripts compiled: {0}", assemblyPath.string());
		return true;
	}

	bool ScriptEngine::LoadHost()
	{
		if (loadAssembly)
			return (managedStart && managedCreate && managedUpdate && managedStop) || ResolveManagedMethods();
		const auto hostPath = HostFxrPath();
		const auto config = ExecutableDirectory() / "Managed" / "Maiden.ScriptHost.runtimeconfig.json";
		if (hostPath.empty() || !std::filesystem::is_regular_file(config))
		{
			MADAM_ERROR("Could not find .NET hostfxr or the managed runtime configuration.");
			return false;
		}
		hostfxr = LoadLibraryW(hostPath.c_str());
		if (!hostfxr) { MADAM_ERROR("Could not load hostfxr: {0}", hostPath.string()); return false; }
		const auto initialize = reinterpret_cast<HostInitialize>(GetProcAddress(hostfxr,
			"hostfxr_initialize_for_runtime_config"));
		const auto getDelegate = reinterpret_cast<HostGetDelegate>(GetProcAddress(hostfxr,
			"hostfxr_get_runtime_delegate"));
		const auto close = reinterpret_cast<HostClose>(GetProcAddress(hostfxr, "hostfxr_close"));
		if (!initialize || !getDelegate || !close)
		{
			MADAM_ERROR("The installed hostfxr is missing required hosting functions.");
			return false;
		}
		void* context = nullptr;
		const int32_t initialized = initialize(config.c_str(), nullptr, &context);
		if (initialized < 0 || !context)
		{
			MADAM_ERROR("Could not initialize .NET runtime: {0}", initialized);
			return false;
		}
		const int32_t delegated = getDelegate(context, 5, &loadAssembly);
		close(context);
		if (delegated < 0 || !loadAssembly)
		{
			MADAM_ERROR("Could not load .NET assembly delegate: {0}", delegated);
			return false;
		}
		if (!ResolveManagedMethods())
		{
			MADAM_ERROR("Could not resolve managed script host methods.");
			return false;
		}
		return true;
	}

	bool ScriptEngine::ResolveManagedMethods()
	{
		const auto assembly = ExecutableDirectory() / "Managed" / "Maiden.ScriptHost.dll";
		if (!std::filesystem::is_regular_file(assembly)) return false;
		const auto load = reinterpret_cast<LoadAssembly>(loadAssembly);
		const auto method = [&](const wchar_t* name, void** pointer)
		{
			return load(assembly.c_str(), L"Maiden.ScriptHost.EntryPoints, Maiden.ScriptHost",
				name, reinterpret_cast<const wchar_t*>(-1), nullptr, pointer) == 0 && *pointer;
		};
		return method(L"Start", reinterpret_cast<void**>(&managedStart)) &&
			method(L"Create", reinterpret_cast<void**>(&managedCreate)) &&
			method(L"Update", reinterpret_cast<void**>(&managedUpdate)) &&
			method(L"Stop", reinterpret_cast<void**>(&managedStop));
	}

	bool ScriptEngine::Start(const std::filesystem::path& projectDirectory, Ref<Scene> scene)
	{
		Stop();
		if (!scene) return false;
		activeScene = scene;
		activeInstance = this;
		auto& manager = Project::Get().getAssetManager();
		auto view = scene->GetRegistry().view<CCSharpScript>();
		if (view.begin() == view.end()) { running = true; return true; }
		std::filesystem::path assembly;
		if (!BuildScripts(projectDirectory, assembly) || !LoadHost()) { Stop(); return false; }
		const NativeCallbacks callbacks{ &GetPosition, &SetPosition, &Log };
		const std::string assemblyName = assembly.u8string();
		if (!managedStart(assemblyName.c_str(), &callbacks)) { Stop(); return false; }
		managedRunning = true;
		for (auto entity : view)
		{
			const auto& script = view.get<CCSharpScript>(entity);
			if (manager.GetAssetType(script.script) != AssetType::SCRIPT)
			{
				MADAM_ERROR("A C# script reference is missing on entity {0}", static_cast<uint32_t>(entity));
				Stop(); return false;
			}
			const auto path = manager.ResolveAssetPath(script.script);
			if (path.extension() != ".cs")
			{
				MADAM_ERROR("The selected script must be a .cs file: {0}", path.string());
				Stop(); return false;
			}
			const std::string className = script.className.empty() ? path.stem().u8string() : script.className;
			if (!managedCreate(static_cast<uint32_t>(entity), className.c_str()))
			{
				Stop(); return false;
			}
		}
		running = true;
		return true;
	}

	void ScriptEngine::Update(float deltaTime)
	{
		if (running && managedRunning && managedUpdate) managedUpdate(deltaTime);
	}

	void ScriptEngine::Stop()
	{
		if (managedRunning && managedStop) managedStop();
		managedRunning = false;
		running = false;
		activeScene.reset();
		if (activeInstance == this) activeInstance = nullptr;
	}

	int __cdecl ScriptEngine::GetPosition(uint32_t entity, float* x, float* y, float* z)
	{
		if (!activeInstance || !activeInstance->activeScene || !x || !y || !z) return 0;
		auto& registry = activeInstance->activeScene->GetRegistry();
		const auto handle = static_cast<entt::entity>(entity);
		if (!registry.valid(handle) || !registry.all_of<CTransform>(handle)) return 0;
		const auto& position = registry.get<CTransform>(handle).translation;
		*x = position.x; *y = position.y; *z = position.z;
		return 1;
	}

	int __cdecl ScriptEngine::SetPosition(uint32_t entity, float x, float y, float z)
	{
		if (!activeInstance || !activeInstance->activeScene) return 0;
		auto& registry = activeInstance->activeScene->GetRegistry();
		const auto handle = static_cast<entt::entity>(entity);
		if (!registry.valid(handle) || !registry.all_of<CTransform>(handle)) return 0;
		registry.get<CTransform>(handle).translation = { x, y, z };
		return 1;
	}

	void __cdecl ScriptEngine::Log(int level, const char* message)
	{
		if (level >= 3)
		{
			MADAM_ERROR("C#: {0}", message ? message : "");
		}
		else if (level == 2)
		{
			MADAM_WARN("C#: {0}", message ? message : "");
		}
		else
		{
			MADAM_INFO("C#: {0}", message ? message : "");
		}
	}
}
