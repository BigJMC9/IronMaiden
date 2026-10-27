#include "maidenpch.hpp"
#include "H_AssetSystem.h"
#include "H_AssetImporter.h"
#include "H_MaterialAsset.h"
#include "Asset.h"
#include "../Core/H_Logger.hpp"
#include "../Rendering/Vulkan/ShaderCompilation/H_VulkanShaderCompiler.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

namespace Madam
{
	namespace
	{
		AssetMetadata s_NullMetadata;
		constexpr const char* editorPrefix = "editor://";

		bool IsEditorHandle(const UUID& handle)
		{
			return ((std::string)handle).rfind(editorPrefix, 0) == 0;
		}

		bool IsChildOrSame(const std::filesystem::path& parent, const std::filesystem::path& child)
		{
			auto parentPart = parent.begin();
			auto childPart = child.begin();
			for (; parentPart != parent.end(); ++parentPart, ++childPart)
			{
				if (childPart == child.end() || *parentPart != *childPart)
					return false;
			}
			return true;
		}

		AssetType TypeForPath(const std::filesystem::path& path)
		{
			std::string extension = path.extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			auto it = assetExtensionMap.find(extension);
			return it == assetExtensionMap.end() ? AssetType::NONE : it->second;
		}

		bool IsCSharpIdentifier(const std::string& value)
		{
			if (value.empty() || !(std::isalpha(static_cast<unsigned char>(value[0])) || value[0] == '_'))
				return false;
			return std::all_of(value.begin() + 1, value.end(), [](unsigned char c)
				{ return std::isalnum(c) || c == '_'; });
		}

		const char* ResourceFolderForType(AssetType type)
		{
			switch (type)
			{
			case AssetType::TEXTURE: return "textures";
			case AssetType::MATERIAL: return "materials";
			case AssetType::SHADER: return "shaders";
			case AssetType::MESH: return "models";
			case AssetType::SCRIPT: return "scripts";
			case AssetType::SCENE: return "scenes";
			default: return "";
			}
		}
	}

	AssetManager::AssetManager(std::filesystem::path projectPath, std::filesystem::path registryLocation)
		: projectPath(std::filesystem::absolute(projectPath).lexically_normal())
	{
		const auto defaultRegistry = this->projectPath / RegistryFile();
		const auto configuredRegistry = registryLocation.empty() ? std::filesystem::path{} :
			(registryLocation.is_absolute() ? registryLocation : this->projectPath / registryLocation).lexically_normal();
		registryPath = !configuredRegistry.empty() && std::filesystem::is_regular_file(configuredRegistry)
			? configuredRegistry : defaultRegistry;
		AssetImporter::Init();
		LoadRegistry();
	}

	AssetManager::~AssetManager() = default;

	std::filesystem::path AssetManager::NormalizeAssetPath(const std::filesystem::path& path, bool recoverLegacy) const
	{
		if (path.empty())
			return {};

		std::filesystem::path relative = path.is_absolute()
			? path.lexically_normal().lexically_relative(projectPath)
			: path.lexically_normal();
		if (!path.is_absolute() && !IsChildOrSame(assets, relative))
			relative = std::filesystem::absolute(path).lexically_normal().lexically_relative(projectPath);

		if ((!IsChildOrSame(assets, relative) || relative.is_absolute()) && recoverLegacy)
		{
			// Older registries wrote absolute paths. Recover the Assets suffix after a project move.
			std::filesystem::path suffix;
			bool foundAssets = false;
			for (const auto& part : path)
			{
				if (part == assets)
				{
					foundAssets = true;
					suffix.clear();
				}
				if (foundAssets)
					suffix /= part;
			}
			relative = suffix.lexically_normal();
		}

		if (relative.empty() || relative.is_absolute() || !IsChildOrSame(assets, relative))
			return {};
		for (const auto& part : relative)
		{
			if (part == "..")
				return {};
		}
		return relative;
	}

	std::filesystem::path AssetManager::GetAbsolutePath(const std::filesystem::path& path) const
	{
		return path.is_absolute() ? path.lexically_normal() : (projectPath / path).lexically_normal();
	}

	std::filesystem::path AssetManager::GetRegistryLocation() const
	{
		const auto relative = registryPath.lexically_relative(projectPath);
		return !relative.empty() && !relative.is_absolute() && !IsChildOrSame("..", relative)
			? relative : registryPath;
	}

	std::filesystem::path AssetManager::ResolveAssetPath(UUID handle) const
	{
		if (IsEditorHandle(handle))
		{
			const auto relative = std::filesystem::u8path(((std::string)handle).substr(std::strlen(editorPrefix))).lexically_normal();
			if (relative.empty() || relative.is_absolute())
				return {};
			for (const auto& part : relative)
				if (part == "..") return {};
			return (std::filesystem::absolute("resources") / relative).lexically_normal();
		}
		if (!m_registry.contains(handle))
			return {};
		return GetAbsolutePath(m_registry.Get(handle).filepath);
	}

	std::vector<AssetReference> AssetManager::ListAssets(AssetType type) const
	{
		std::vector<AssetReference> result;
		for (const auto& [handle, metadata] : m_registry)
		{
			if (metadata.assetType == type && !metadata.isMissing &&
				(type != AssetType::SHADER || metadata.filepath.extension() == ".frag"))
				result.push_back({ handle, type, GetAbsolutePath(metadata.filepath), false });
		}
		const auto resourceFolder = ResourceFolderForType(type);
		if (resourceFolder[0] != '\0')
		{
			const auto root = std::filesystem::absolute("resources");
			const auto directory = root / resourceFolder;
			std::error_code error;
			std::filesystem::recursive_directory_iterator it(directory, error), end;
			for (; it != end && !error; it.increment(error))
			{
				if (!it->is_regular_file(error) || error)
					continue;
				if (TypeForPath(it->path()) != type ||
					(type == AssetType::SHADER && it->path().extension() != ".frag"))
					continue;
				const auto relative = it->path().lexically_relative(root);
				result.push_back({ UUID(std::string(editorPrefix) + relative.generic_string()), type, it->path(), true });
			}
		}
		std::sort(result.begin(), result.end(), [](const AssetReference& a, const AssetReference& b)
			{ return a.path.filename().string() < b.path.filename().string(); });
		return result;
	}

	ShaderDefinition AssetManager::GetShaderDefinition(UUID shader) const
	{
		const auto source = ResolveAssetPath(shader);
		if (source.empty() || !std::filesystem::is_regular_file(source))
		{
			shaderDefinitions.erase(shader);
			ShaderDefinition result;
			result.error = "The selected shader is missing.";
			return result;
		}
		const auto defaultVertex = std::filesystem::absolute("resources/shaders/texture_shader.vert");
		auto companionVertex = source;
		companionVertex.replace_extension(".vert");
		const auto vertex = std::filesystem::is_regular_file(companionVertex)
			? companionVertex : defaultVertex;
		const auto stamp = [](const std::filesystem::path& path) {
			std::error_code error;
			const auto time = std::filesystem::last_write_time(path, error);
			return error ? std::filesystem::file_time_type{} : time;
		};
		const auto size = [](const std::filesystem::path& path) {
			std::error_code error;
			const auto bytes = std::filesystem::file_size(path, error);
			return error ? uintmax_t{ 0 } : bytes;
		};
		const auto sourceTime = stamp(source);
		const auto vertexTime = stamp(vertex);
		const auto sourceSize = size(source);
		const auto vertexSize = size(vertex);
		const auto cached = shaderDefinitions.find(shader);
		if (cached != shaderDefinitions.end() && cached->second.source == source &&
			cached->second.vertex == vertex && cached->second.sourceTime == sourceTime &&
			cached->second.vertexTime == vertexTime && cached->second.sourceSize == sourceSize &&
			cached->second.vertexSize == vertexSize)
			return cached->second.definition;
		auto definition = ReadMaterialShader(source, defaultVertex);
		shaderDefinitions[shader] = { source, vertex, sourceTime, vertexTime,
			sourceSize, vertexSize, definition };
		return definition;
	}

	AssetType AssetManager::GetAssetType(UUID uuid)
	{
		if (IsEditorHandle(uuid))
		{
			const auto path = ResolveAssetPath(uuid);
			return !path.empty() && std::filesystem::is_regular_file(path) ? TypeForPath(path) : AssetType::NONE;
		}
		return IsAssetHandleValid(uuid) ? GetMetadata(uuid).assetType : AssetType::NONE;
	}

	Ref<Asset> AssetManager::GetAsset(UUID uuid)
	{
		if (IsEditorHandle(uuid))
		{
			const AssetType type = GetAssetType(uuid);
			if (type != AssetType::TEXTURE && type != AssetType::MATERIAL && type != AssetType::MESH &&
				type != AssetType::SHADER && type != AssetType::SCRIPT)
				return nullptr;
			const auto cached = loadedAssets.find(uuid);
			if (cached != loadedAssets.end())
				return cached->second;
			AssetMetadata builtIn;
			builtIn.uuid = uuid;
			builtIn.assetType = type;
			builtIn.filepath = ResolveAssetPath(uuid);
			Ref<Asset> asset;
			if (type == AssetType::SHADER || type == AssetType::SCRIPT)
				asset = CreateRef<SourceAsset>(type);
			else if (!AssetImporter::tryLoadData(builtIn, asset) || !asset)
				return nullptr;
			asset->handle = uuid;
			loadedAssets[uuid] = asset;
			return asset;
		}
		AssetMetadata& metadata = GetInternalMetadata(uuid);
		if (!metadata.IsValid())
			return nullptr;
		const auto filePath = GetAbsolutePath(metadata.filepath);
		if (!std::filesystem::is_regular_file(filePath))
		{
			metadata.isMissing = true;
			metadata.isLoaded = false;
			loadedAssets.erase(uuid);
			return nullptr;
		}
		metadata.isMissing = false;

		auto cached = loadedAssets.find(uuid);
		if (cached != loadedAssets.end())
			return cached->second;
		if (metadata.assetType == AssetType::SHADER || metadata.assetType == AssetType::SCRIPT)
		{
			auto source = CreateRef<SourceAsset>(metadata.assetType);
			source->handle = uuid;
			metadata.isLoaded = true;
			loadedAssets[uuid] = source;
			return source;
		}

		AssetMetadata loadMetadata = metadata;
		loadMetadata.filepath = filePath;

		Ref<Asset> asset;
		if (!AssetImporter::tryLoadData(loadMetadata, asset) || !asset)
			return nullptr;
		asset->handle = uuid;
		metadata.isLoaded = true;
		loadedAssets[uuid] = asset;
		return asset;
	}

	Ref<MaterialAsset> AssetManager::CreateMaterial(const std::filesystem::path& path)
	{
		const auto relative = NormalizeAssetPath(path);
		if (relative.empty() || TypeForPath(relative) != AssetType::MATERIAL ||
			GetMetadata(relative).IsValid() || std::filesystem::exists(GetAbsolutePath(relative)))
			return nullptr;

		AssetMetadata metadata;
		metadata.uuid = UUID();
		metadata.assetType = AssetType::MATERIAL;
		metadata.filepath = relative;
		auto material = CreateRef<MaterialAsset>();
		material->handle = metadata.uuid;

		std::filesystem::create_directories(GetAbsolutePath(relative).parent_path());
		AssetMetadata writeMetadata = metadata;
		writeMetadata.filepath = GetAbsolutePath(relative);
		if (!AssetImporter::Serialize(writeMetadata, material))
			return nullptr;

		metadata.isLoaded = true;
		m_registry[metadata.uuid] = metadata;
		loadedAssets[metadata.uuid] = material;
		if (!SaveMetaData())
		{
			RemoveMetadata(metadata.uuid);
			std::error_code error;
			std::filesystem::remove(writeMetadata.filepath, error);
			MADAM_CORE_ERROR("Material could not be registered: {0}", writeMetadata.filepath.string());
			return nullptr;
		}
		return material;
	}

	Ref<SourceAsset> AssetManager::CreateShader(const std::filesystem::path& fragmentPath)
	{
		const auto relative = NormalizeAssetPath(fragmentPath);
		if (relative.empty() || relative.extension() != ".frag" ||
			GetMetadata(relative).IsValid()) return nullptr;
		auto vertex = relative;
		vertex.replace_extension(".vert");
		const auto fragmentFile = GetAbsolutePath(relative);
		const auto vertexFile = GetAbsolutePath(vertex);
		if (std::filesystem::exists(fragmentFile) || std::filesystem::exists(vertexFile)) return nullptr;
		const auto templateRoot = std::filesystem::absolute("resources/shaders");
		std::error_code error;
		std::filesystem::create_directories(fragmentFile.parent_path(), error);
		if (error) return nullptr;
		std::filesystem::copy_file(templateRoot / "texture_shader.frag", fragmentFile, error);
		if (error) return nullptr;
		std::filesystem::copy_file(templateRoot / "texture_shader.vert", vertexFile, error);
		if (error)
		{
			std::filesystem::remove(fragmentFile, error);
			return nullptr;
		}
		AppendMetaData(vertexFile);
		AppendMetaData(fragmentFile);
		const auto& metadata = GetMetadata(fragmentFile);
		if (!metadata.IsValid())
		{
			RemoveMetadata(vertexFile);
			SaveMetaData();
			std::filesystem::remove(fragmentFile, error);
			std::filesystem::remove(vertexFile, error);
			return nullptr;
		}
		return std::dynamic_pointer_cast<SourceAsset>(GetAsset(metadata.uuid));
	}

	Ref<SourceAsset> AssetManager::CreateCSharpScript(const std::filesystem::path& path)
	{
		const auto relative = NormalizeAssetPath(path);
		if (relative.empty() || relative.extension() != ".cs" ||
			!IsCSharpIdentifier(relative.stem().string()) || GetMetadata(relative).IsValid())
			return nullptr;
		const auto source = GetAbsolutePath(relative);
		if (std::filesystem::exists(source)) return nullptr;
		std::error_code error;
		std::filesystem::create_directories(source.parent_path(), error);
		if (error) return nullptr;
		std::ofstream file(source, std::ios::trunc);
		if (!file) return nullptr;
		file << "using Maiden;\n\npublic class " << relative.stem().string() << " : Behaviour\n"
			<< "{\n    public override void OnCreate() { }\n\n"
			<< "    public override void OnUpdate(float deltaTime) { }\n\n"
			<< "    public override void OnDestroy() { }\n}\n";
		file.close();
		if (!file.good()) { std::filesystem::remove(source, error); return nullptr; }
		AppendMetaData(source);
		const auto& metadata = GetMetadata(source);
		if (!metadata.IsValid()) { std::filesystem::remove(source, error); return nullptr; }
		return std::dynamic_pointer_cast<SourceAsset>(GetAsset(metadata.uuid));
	}

	bool AssetManager::CompileShader(UUID shader, std::string* diagnostic) const
	{
		shaderDefinitions.erase(shader);
		auto fail = [&](const std::string& message)
		{
			if (diagnostic) *diagnostic = message;
			return false;
		};
		const auto metadata = m_registry.contains(shader) ? m_registry.Get(shader) : AssetMetadata{};
		if (!metadata.IsValid() || metadata.assetType != AssetType::SHADER ||
			metadata.filepath.extension() != ".frag")
			return fail("Select a project fragment shader (.frag).");
		const auto definition = GetShaderDefinition(shader);
		if (!definition.IsValid()) return fail(definition.error);
		std::string message;
		const auto vertex = VulkanShaderCompiler::CompileStage(definition.vertexPath,
			VK_SHADER_STAGE_VERTEX_BIT, &message);
		if (vertex.empty()) return fail(message);
		const auto fragment = VulkanShaderCompiler::CompileStage(definition.fragmentPath,
			VK_SHADER_STAGE_FRAGMENT_BIT, &message);
		if (fragment.empty()) return fail(message);
		const auto output = projectPath / "Library" / "Shaders";
		std::error_code error;
		std::filesystem::create_directories(output, error);
		if (error) return fail(error.message());
		const auto name = (std::string)shader;
		const auto write = [&](const std::filesystem::path& path, const std::vector<uint32_t>& code)
		{
			std::ofstream file(path, std::ios::binary | std::ios::trunc);
			file.write(reinterpret_cast<const char*>(code.data()),
				static_cast<std::streamsize>(code.size() * sizeof(uint32_t)));
			return file.good();
		};
		if (!write(output / (name + ".vert.spv"), vertex) ||
			!write(output / (name + ".frag.spv"), fragment))
			return fail("Could not write the compiled shader to Library/Shaders.");
		if (diagnostic) *diagnostic = "Compiled vertex and fragment stages.";
		return true;
	}

	bool AssetManager::SaveAsset(const Ref<Asset>& asset)
	{
		if (!asset)
			return false;
		const AssetMetadata& metadata = GetMetadata(asset->handle);
		if (!metadata.IsValid() || metadata.isMissing || metadata.assetType != asset->GetAssetType())
			return false;
		AssetMetadata writeMetadata = metadata;
		writeMetadata.filepath = GetAbsolutePath(metadata.filepath);
		if (!AssetImporter::Serialize(writeMetadata, asset))
			return false;
		loadedAssets[asset->handle] = asset;
		m_registry[asset->handle].isLoaded = true;
		return true;
	}

	const AssetMetadata& AssetManager::GetMetadata(UUID uuid)
	{
		return m_registry.contains(uuid) ? m_registry[uuid] : s_NullMetadata;
	}

	const AssetMetadata& AssetManager::GetMetadata(std::filesystem::path path)
	{
		const auto relative = NormalizeAssetPath(path);
		if (relative.empty())
			return s_NullMetadata;
		const auto found = pathIndex.find(relative);
		return found == pathIndex.end() ? s_NullMetadata : GetMetadata(found->second);
	}

	const AssetMetadata& AssetManager::GetMetadata(const Ref<Asset>& asset)
	{
		return asset ? GetMetadata(asset->handle) : s_NullMetadata;
	}

	AssetMetadata& AssetManager::GetInternalMetadata(UUID uuid)
	{
		return m_registry.contains(uuid) ? m_registry[uuid] : s_NullMetadata;
	}

	void AssetManager::RemoveMetadata(UUID uuid)
	{
		if (m_registry.contains(uuid))
			pathIndex.erase(m_registry.Get(uuid).filepath);
		loadedAssets.erase(uuid);
		m_registry.remove(uuid);
	}

	void AssetManager::RemoveMetadata(std::filesystem::path path)
	{
		const auto relative = NormalizeAssetPath(path);
		if (relative.empty())
			return;
		std::vector<UUID> removed;
		for (const auto& [handle, metadata] : m_registry)
		{
			if (IsChildOrSame(relative, metadata.filepath))
				removed.push_back(handle);
		}
		for (const auto& handle : removed)
			RemoveMetadata(handle);
	}

	bool AssetManager::MoveMetadata(const std::filesystem::path& oldPath, const std::filesystem::path& newPath)
	{
		const auto oldRelative = NormalizeAssetPath(oldPath);
		const auto newRelative = NormalizeAssetPath(newPath);
		if (oldRelative.empty() || newRelative.empty())
			return false;

		std::vector<std::pair<UUID, std::filesystem::path>> changes;
		for (const auto& [handle, metadata] : m_registry)
		{
			if (!IsChildOrSame(oldRelative, metadata.filepath))
				continue;
			const auto suffix = metadata.filepath.lexically_relative(oldRelative);
			const auto destination = suffix.empty() || suffix == "." ? newRelative : newRelative / suffix;
			if (TypeForPath(destination) != metadata.assetType)
				return false;
			changes.emplace_back(handle, destination.lexically_normal());
		}
		for (const auto& [handle, destination] : changes)
		{
			auto& metadata = m_registry[handle];
			pathIndex.erase(metadata.filepath);
			metadata.filepath = destination;
			pathIndex[metadata.filepath] = handle;
			metadata.isMissing = false;
			metadata.isLoaded = false;
			loadedAssets.erase(handle);
		}
		return !changes.empty();
	}

	void AssetManager::RebuildPathIndex()
	{
		pathIndex.clear();
		pathIndex.reserve(m_registry.count());
		for (const auto& [handle, metadata] : m_registry)
			pathIndex[metadata.filepath] = handle;
	}

	void AssetManager::LoadRegistry()
	{
		std::ifstream file(registryPath);
		if (file && !m_registry.Deserialize(file))
		{
			MADAM_CORE_ERROR("Could not read asset registry: {0}", registryPath.string());
			registryWritable = false;
		}
		file.close();

		std::vector<UUID> invalid;
		for (auto& [handle, metadata] : m_registry)
		{
			const auto relative = NormalizeAssetPath(metadata.filepath, true);
			if (relative.empty() || metadata.assetType == AssetType::NONE ||
				TypeForPath(relative) != metadata.assetType)
			{
				invalid.push_back(handle);
				continue;
			}
			metadata.filepath = relative;
			metadata.isLoaded = false;
		}
		for (const auto& handle : invalid)
			RemoveMetadata(handle);
		RebuildPathIndex();

		if (ScanAssets() && registryWritable)
			SaveMetaData();
	}

	bool AssetManager::SaveMetaData()
	{
		if (!registryWritable)
		{
			MADAM_CORE_ERROR("Asset registry cannot be saved because it could not be loaded: {0}", registryPath.string());
			return false;
		}
		const auto tempPath = std::filesystem::path(registryPath.wstring() + L".tmp");
		std::error_code error;
		std::filesystem::create_directories(registryPath.parent_path(), error);
		if (error)
		{
			MADAM_CORE_ERROR("Cannot create asset registry directory: {0}", error.message());
			return false;
		}
		std::ofstream file(tempPath, std::ios::binary | std::ios::trunc);
		if (!file)
		{
			MADAM_CORE_ERROR("Cannot open temporary asset registry: {0}", tempPath.string());
			return false;
		}
		const bool written = m_registry.Serialize(file);
		file.close();
		if (!written)
		{
			MADAM_CORE_ERROR("Cannot write temporary asset registry: {0}", tempPath.string());
			std::filesystem::remove(tempPath, error);
			return false;
		}
		if (!MoveFileExW(tempPath.c_str(), registryPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			MADAM_CORE_ERROR("Cannot replace asset registry {0}: Win32 error {1}", registryPath.string(), GetLastError());
			std::filesystem::remove(tempPath, error);
			return false;
		}
		return true;
	}

	bool AssetManager::ScanAssets()
	{
		const auto assetPath = projectPath / assets;
		if (!std::filesystem::is_directory(assetPath))
		{
			MADAM_CORE_ERROR("Asset path does not exist: {0}", assetPath.string());
			return false;
		}

		for (auto& [handle, metadata] : m_registry)
			metadata.isMissing = true;

		std::error_code error;
		std::filesystem::recursive_directory_iterator it(assetPath, error);
		std::filesystem::recursive_directory_iterator end;
		for (; it != end && !error; it.increment(error))
		{
			if (!it->is_regular_file(error) || error)
				continue;
			const auto relative = NormalizeAssetPath(it->path());
			if (relative.empty() || TypeForPath(relative) == AssetType::NONE)
				continue;
			const auto& metadata = GetMetadata(relative);
			if (metadata.IsValid())
				m_registry[metadata.uuid].isMissing = false;
			else
				SetMetaData(relative);
		}
		if (error)
		{
			MADAM_CORE_ERROR("Asset scan failed: {0}", error.message());
		}
		return !error;
	}

	void AssetManager::AppendMetaData(const std::filesystem::path& path)
	{
		const auto relative = NormalizeAssetPath(path);
		if (!relative.empty() && TypeForPath(relative) != AssetType::NONE &&
			!GetMetadata(relative).IsValid() &&
			std::filesystem::is_regular_file(GetAbsolutePath(relative)))
		{
			SetMetaData(relative);
			const UUID handle = GetMetadata(relative).uuid;
			if (!SaveMetaData())
			{
				RemoveMetadata(handle);
				MADAM_CORE_ERROR("Could not save metadata for asset: {0}", relative.string());
			}
		}
	}

	void AssetManager::SetMetaData(const std::filesystem::path& path)
	{
		const AssetType type = TypeForPath(path);
		if (type == AssetType::NONE)
			return;
		AssetMetadata metadata;
		metadata.filepath = path;
		metadata.uuid = UUID();
		metadata.assetType = type;
		m_registry[metadata.uuid] = metadata;
		pathIndex[metadata.filepath] = metadata.uuid;
	}
}
