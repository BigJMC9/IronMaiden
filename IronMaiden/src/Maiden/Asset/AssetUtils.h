#pragma once
#include "maidenpch.hpp"

namespace Madam::Asset
{
	const std::string assets = "Assets";
	const std::string resource = "Resources";
	const std::string metadata = "Resources";
	const std::string metadataExtension = ".mmeta";

	inline std::string registryFile() {
		return resource + "\\" + metadata + metadataExtension;
	}

	enum class AssetType
	{
		NONE,
		TEXTURE,
		MATERIAL,
		SCENE,
		SHADER,
		SCRIPT,
		MESH,
	};

	inline std::string assetTypeToString(AssetType type) {
		switch (type)
		{
		case AssetType::NONE: return "None";
		case AssetType::TEXTURE: return "Texture";
		case AssetType::MATERIAL: return "Material";
		case AssetType::SCENE: return "Scene";
		case AssetType::SHADER: return "Shader";
		case AssetType::SCRIPT: return "Script";
		case AssetType::MESH: return "Mesh";
		default: return "None";
		}
	}

	inline static std::unordered_map<std::string, AssetType> assetExtensionMap = {
		{".png", AssetType::TEXTURE},
		{".jpg", AssetType::TEXTURE},
		{".jpeg", AssetType::TEXTURE},
		{".bmp", AssetType::TEXTURE},
		{".mat", AssetType::MATERIAL},
		{".scene", AssetType::SCENE},
		{".glsl", AssetType::SHADER},
		{".vert", AssetType::SHADER},
		{".frag", AssetType::SHADER},
		{".tesc", AssetType::SHADER},
		{".tese", AssetType::SHADER},
		{".geom", AssetType::SHADER},
		{".hlsl", AssetType::SHADER},
		{".vs", AssetType::SHADER},
		{".ps", AssetType::SHADER},
		{".gs", AssetType::SHADER},
		{".hs", AssetType::SHADER},
		{".ds", AssetType::SHADER},
		{".cpp", AssetType::SCRIPT},
		{".obj", AssetType::MESH},
		{".fbx", AssetType::MESH},
	};

	inline static std::unordered_map<std::string, AssetType> assetTypeMap = {
		{"None", AssetType::NONE},
		{"Texture", AssetType::TEXTURE},
		{"Material", AssetType::MATERIAL},
		{"Scene", AssetType::SCENE},
		{"Shader", AssetType::SHADER},
		{"Script", AssetType::SCRIPT},
		{"Mesh", AssetType::MESH},
	};
}