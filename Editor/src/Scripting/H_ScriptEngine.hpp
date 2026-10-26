#pragma once

#include <Maiden.hpp>
#include <filesystem>
#include <string>

namespace Madam::Scripting
{
	class ScriptEngine
	{
	public:
		ScriptEngine() = default;
		~ScriptEngine();

		bool Start(const std::filesystem::path& projectDirectory, Ref<Scene> scene);
		void Update(float deltaTime);
		void Stop();
		bool IsRunning() const { return running; }

	private:
		struct NativeCallbacks
		{
			int(__cdecl* getPosition)(uint32_t, float*, float*, float*);
			int(__cdecl* setPosition)(uint32_t, float, float, float);
			void(__cdecl* log)(int, const char*);
		};

		using ManagedStart = int(__cdecl*)(const char*, const NativeCallbacks*);
		using ManagedCreate = int(__cdecl*)(uint32_t, const char*);
		using ManagedUpdate = void(__cdecl*)(float);
		using ManagedStop = void(__cdecl*)();

		bool BuildScripts(const std::filesystem::path& projectDirectory,
			std::filesystem::path& assemblyPath);
		bool LoadHost();
		bool ResolveManagedMethods();
		static int __cdecl GetPosition(uint32_t entity, float* x, float* y, float* z);
		static int __cdecl SetPosition(uint32_t entity, float x, float y, float z);
		static void __cdecl Log(int level, const char* message);

		static ScriptEngine* activeInstance;
		Ref<Scene> activeScene;
		HMODULE hostfxr = nullptr;
		void* loadAssembly = nullptr;
		ManagedStart managedStart = nullptr;
		ManagedCreate managedCreate = nullptr;
		ManagedUpdate managedUpdate = nullptr;
		ManagedStop managedStop = nullptr;
		bool running = false;
		bool managedRunning = false;
	};
}
