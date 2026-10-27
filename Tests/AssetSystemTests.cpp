#include "Maiden/Asset/H_AssetSystem.h"
#include "Maiden/Asset/H_MaterialAsset.h"
#include "Maiden/Core/H_Logger.hpp"
#include "Maiden/Rendering/Vulkan/ShaderCompilation/H_VulkanShaderCompiler.h"
#include "Maiden/Project/H_Project.h"
#include "Maiden/Scene/H_Scene.hpp"
#include "Maiden/Scene/Components.hpp"
#include "../Editor/src/Scripting/H_ScriptEngine.hpp"

#include <filesystem>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace Madam;

static bool Check(bool condition, const char* message)
{
	if (!condition)
		std::cerr << message << '\n';
	return condition;
}

struct NativeScriptProbe : ScriptableEntity
{
	static inline int destroyed = 0;
	void Create() {}
	void Start() {}
	void Update() {}
	void LateUpdate() {}
	void Render() {}
	void Destroy() { ++destroyed; }
};

int main()
{
	const auto temporary = std::filesystem::temp_directory_path();
	const auto testRoot = temporary / ("IronMaidenAssetTest-" + (std::string)Madam::UUID());
	const auto originalProject = testRoot / "Original";
	const auto movedProject = testRoot / "Moved";
	const auto relativeMaterial = std::filesystem::path("Assets") / "Sample.mat";
	const auto renamedMaterial = std::filesystem::path("Assets") / "Renamed.mat";
	const auto previousDirectory = std::filesystem::current_path();
	const auto standardShader = ReadMaterialShader(previousDirectory / "Editor" / "resources" / "shaders" / "texture_shader.frag",
		previousDirectory / "Editor" / "resources" / "shaders" / "texture_shader.vert");
	if (!Check(standardShader.IsValid() && standardShader.textures.size() == 4 &&
		standardShader.textures[0].name == "diffuseMap" && standardShader.textures[3].binding == 3,
		"Standard shader inputs were not discovered")) return 1;
	std::filesystem::create_directories(originalProject / "Assets");
	std::filesystem::create_directories(originalProject / "Resources");
	std::filesystem::create_directories(testRoot / "logs");
	std::filesystem::create_directories(testRoot / "resources" / "textures");
	std::filesystem::create_directories(testRoot / "resources" / "shaders");
	std::filesystem::create_directories(testRoot / "resources" / "models");
	std::ofstream(testRoot / "resources" / "textures" / "EditorTexture.png") << "texture data";
	std::ofstream(testRoot / "resources" / "models" / "EditorMesh.obj") << "# editor mesh\n";
	std::filesystem::copy_file(previousDirectory / "Editor" / "resources" / "shaders" / "texture_shader.vert",
		testRoot / "resources" / "shaders" / "texture_shader.vert");
	std::filesystem::copy_file(previousDirectory / "Editor" / "resources" / "shaders" / "texture_shader.frag",
		testRoot / "resources" / "shaders" / "texture_shader.frag");
	std::ofstream(testRoot / "resources" / "shaders" / "Custom.vert")
		<< "#version 450\nvoid main() { gl_Position = vec4(0.0); }\n";
	std::ofstream(testRoot / "resources" / "shaders" / "Custom.frag")
		<< "#version 450\nlayout(set=1, binding=5) uniform sampler2D detailMap;\n"
		<< "layout(binding=2, set=1) uniform sampler2D albedoMap;\n"
		<< "layout(location=0) out vec4 outColor;\nvoid main() { outColor = texture(albedoMap, vec2(0.5)); }\n";
	std::filesystem::current_path(testRoot);
	Logger::Init();
	{
		NativeScriptProbe::destroyed = 0;
		auto scene = CreateRef<Scene>();
		auto entity = scene->CreateEntity("Unload probe");
		entity.AddComponent<CNativeScript>().Bind<NativeScriptProbe>();
		scene->Update();
	}
	if (!Check(NativeScriptProbe::destroyed == 1,
		"Unloading a scene did not destroy its native script")) return 1;
	{
		auto scene = CreateRef<Scene>();
		auto entity = scene->CreateEntity("Delete probe");
		entity.AddComponent<CNativeScript>().Bind<NativeScriptProbe>();
		scene->Update();
		scene->DestroyEntity(entity);
	}
	if (!Check(NativeScriptProbe::destroyed == 2,
		"Deleting an entity did not destroy its native script exactly once")) return 1;
	{
		auto scene = CreateRef<Scene>();
		auto entity = scene->CreateEntity("Component probe");
		entity.AddComponent<CNativeScript>().Bind<NativeScriptProbe>();
		scene->Update();
		entity.RemoveComponent<CNativeScript>();
	}
	if (!Check(NativeScriptProbe::destroyed == 3,
		"Removing a native script component did not destroy its instance")) return 1;
	{
		auto scene = CreateRef<Scene>();
		auto sun = scene->CreateEntity("Sun");
		sun.AddComponent<CDirectionalLight>().intensity = 2.5f;
		auto spot = scene->CreateEntity("Spot");
		spot.AddComponent<CSpotLight>().outerAngle = 35.0f;
		auto point = scene->CreateEntity("Point");
		point.AddComponent<CPointLight>().range = 24.0f;
		auto& lighting = scene->GetLightingSettings();
		lighting.shadowMode = ShadowMode::Baked;
		lighting.shadowCaster = sun.GetComponent<CUniqueIdentifier>().uuid;
		lighting.shadowDistance = 80.0f;
		lighting.bakeFile = "Library/Lighting/test.shadowbin";
		Rendering::RenderScene renderScene;
		scene->BuildRenderScene(renderScene);
		if (!Check(renderScene.directionalLightEntities.size() == 1 &&
			renderScene.spotLightEntities.size() == 1 &&
			renderScene.pointLightEntities.size() == 1,
			"Scene render data omitted a light type")) return 1;
		auto copy = scene->Copy();
		const auto& copiedLighting = copy->GetLightingSettings();
		if (!Check(copiedLighting.shadowMode == ShadowMode::Baked &&
			copiedLighting.shadowCaster == lighting.shadowCaster &&
			copiedLighting.shadowDistance == 80.0f &&
			copiedLighting.bakeFile == lighting.bakeFile &&
			copy->GetEntity(sun.GetComponent<CUniqueIdentifier>().uuid).GetComponent<CDirectionalLight>().intensity == 2.5f &&
			copy->GetEntity(spot.GetComponent<CUniqueIdentifier>().uuid).GetComponent<CSpotLight>().outerAngle == 35.0f &&
			copy->GetEntity(point.GetComponent<CUniqueIdentifier>().uuid).GetComponent<CPointLight>().range == 24.0f,
			"Scene copy lost lighting settings or components")) return 1;
	}
	{
		auto scene = CreateRef<Scene>();
		auto editorCamera = scene->GetMainCameraEntity();
		if (!Check(editorCamera && editorCamera.HasComponent<CCamera>(),
			"A scene without a camera did not get an editor camera")) return 1;
		if (!Check(editorCamera.GetComponent<CCamera>().editorOnly && !scene->GetGameCameraEntity(),
			"Editor camera was exposed as a game camera")) return 1;
		auto gameplayCamera = scene->CreateEntity("Gameplay Camera");
		gameplayCamera.AddComponent<CCamera>();
		if (!Check(scene->GetGameCameraEntity() == gameplayCamera,
			"Scene camera was not selected for the Game view")) return 1;
		auto runtimeScene = scene->Copy();
		auto runtimeCamera = runtimeScene->GetMainCameraEntity();
		if (!Check(runtimeCamera && runtimeCamera.HasComponent<CCamera>() &&
			runtimeCamera.GetComponent<CCamera>().cameraHandle != editorCamera.GetComponent<CCamera>().cameraHandle &&
			runtimeCamera.GetComponent<CCamera>().cameraHandle->IsMain(),
			"Copied scene did not select its own camera")) return 1;
		if (!Check(runtimeScene->GetGameCameraEntity() &&
			!runtimeScene->GetGameCameraEntity().GetComponent<CCamera>().editorOnly,
			"Scene copy lost its gameplay camera")) return 1;
		if (!Check(scene->GetMainCameraEntity().GetComponent<CCamera>().cameraHandle->IsMain(),
			"Editor scene camera was not restored")) return 1;
	}
	if (!Check(!VulkanShaderCompiler::CompileStage(standardShader.fragmentPath, VK_SHADER_STAGE_FRAGMENT_BIT).empty() &&
		!VulkanShaderCompiler::CompileStage(standardShader.vertexPath, VK_SHADER_STAGE_VERTEX_BIT).empty(),
		"Standard material shader could not compile")) return 1;

	Madam::UUID handle("");
	{
		AssetManager assets(originalProject);
		auto material = assets.CreateMaterial(originalProject / relativeMaterial);
		if (!Check(material != nullptr, "Could not create a material asset")) return 1;
		handle = material->handle;
		material->textures["diffuseMap"] = Madam::UUID("diffuse-test-id");
		if (!Check(assets.SaveAsset(material), "Could not save a material asset")) return 1;
		if (!Check(assets.GetMetadata(handle).filepath == relativeMaterial, "Registry stored a non-relative path")) return 1;
	}

	{
		AssetManager assets(originalProject);
		if (!Check(assets.GetMetadata(relativeMaterial).uuid == handle, "Material ID changed after reopen")) return 1;
		if (!Check(assets.GetMetadata(std::filesystem::path("Original") / relativeMaterial).uuid == handle,
			"Project-relative browser path could not find metadata")) return 1;
		auto material = std::dynamic_pointer_cast<MaterialAsset>(assets.GetAsset(handle));
		if (!Check(material && material->textures.at("diffuseMap") == Madam::UUID("diffuse-test-id"), "Material data did not round trip")) return 1;

		std::filesystem::rename(originalProject / relativeMaterial, originalProject / renamedMaterial);
		if (!Check(assets.MoveMetadata(originalProject / relativeMaterial, originalProject / renamedMaterial), "Could not move material metadata")) return 1;
		if (!Check(assets.SaveMetaData(), "Could not save moved metadata")) return 1;

		std::filesystem::rename(originalProject / renamedMaterial, testRoot / "hidden.mat");
		if (!Check(assets.ScanAssets() && assets.GetMetadata(handle).isMissing, "Missing file was not detected")) return 1;
		std::filesystem::rename(testRoot / "hidden.mat", originalProject / renamedMaterial);
		if (!Check(assets.ScanAssets() && !assets.GetMetadata(handle).isMissing, "Restored file lost its ID")) return 1;
		if (!Check(assets.SaveMetaData(), "Could not save restored metadata")) return 1;
	}

	std::filesystem::rename(originalProject, movedProject);
	{
		AssetManager assets(movedProject);
		if (!Check(assets.GetMetadata(renamedMaterial).uuid == handle, "Project move changed the material ID")) return 1;
		if (!Check(std::dynamic_pointer_cast<MaterialAsset>(assets.GetAsset(handle)) != nullptr,
			"Material could not be loaded after moving the project")) return 1;
	}

	// A registry from an older project contains absolute paths. It must migrate on open.
	const Madam::UUID ignoredHandle;
	{
		std::ofstream registry(movedProject / "Resources" / "Resources.mmeta", std::ios::trunc);
		registry << "m_registry:\n  - {UUID: '" << (std::string)handle
			<< "', Type: Material, File: '" << (originalProject / renamedMaterial).string()
			<< "', IsGPUObject: false}\n  - {UUID: '" << (std::string)ignoredHandle
			<< "', Type: None, File: '" << (originalProject / "Assets" / "imgui.ini").string()
			<< "', IsGPUObject: false}\n";
	}
	{
		AssetManager assets(movedProject);
		if (!Check(assets.GetMetadata(renamedMaterial).uuid == handle, "Legacy absolute path lost its ID")) return 1;
		if (!Check(assets.GetMetadata(handle).filepath == renamedMaterial, "Legacy path was not made relative")) return 1;
		if (!Check(!assets.GetMetadata(ignoredHandle).IsValid(), "Legacy non-asset entry was not removed")) return 1;
		std::ofstream(movedProject / "Assets" / "ignored.bin") << "ignored";
		if (!Check(assets.ScanAssets() && !assets.GetMetadata(movedProject / "Assets" / "ignored.bin").IsValid(),
			"Unknown file type entered the registry")) return 1;
		auto first = assets.CreateMaterial(movedProject / "Assets" / "Nested" / "First.mat");
		auto second = assets.CreateMaterial(movedProject / "Assets" / "Nested" / "Second.mat");
		if (!Check(first && second, "Could not create materials for directory removal")) return 1;
		assets.RemoveMetadata(movedProject / "Assets" / "Nested");
		if (!Check(!assets.GetMetadata(first->handle).IsValid() && !assets.GetMetadata(second->handle).IsValid(),
			"Directory removal left registry entries behind")) return 1;
		std::ofstream(movedProject / "Assets" / "Legacy.mat")
			<< "Material:\n  Version: 1\n  Textures:\n    Diffuse: legacy-texture\n    Normal: ''\n    AO: ''\n    Gloss: ''\n";
		assets.AppendMetaData(movedProject / "Assets" / "Legacy.mat");
		auto legacy = std::dynamic_pointer_cast<MaterialAsset>(
			assets.GetAsset(assets.GetMetadata(movedProject / "Assets" / "Legacy.mat").uuid));
		if (!Check(legacy && legacy->textures.at("diffuseMap") == Madam::UUID("legacy-texture") &&
			legacy->shader == Madam::UUID("editor://shaders/texture_shader.frag"),
			"Older material inputs were not migrated")) return 1;
	}

	const auto customProject = testRoot / "Custom";
	const auto customRegistry = std::filesystem::path("Resources") / "Custom.mmeta";
	const auto importedTexture = std::filesystem::path("Assets") / "Imported.png";
	std::filesystem::create_directories(customProject / "Assets");
	std::filesystem::create_directories(customProject / "Resources");
	std::ofstream(customProject / customRegistry) << "m_registry: []";
	Madam::UUID textureHandle("");
	{
		AssetManager assets(customProject, customRegistry);
		const auto shader = assets.GetShaderDefinition(Madam::UUID("editor://shaders/Custom.frag"));
		if (!Check(shader.IsValid() && shader.textures.size() == 2 &&
			shader.textures[0].name == "albedoMap" && shader.textures[1].binding == 5,
			"Custom shader bindings were not discovered")) return 1;
		std::ofstream(testRoot / "resources" / "shaders" / "Custom.frag", std::ios::app)
			<< "layout(set=1, binding=6) uniform sampler2D extraMap;\n";
		const auto updatedShader = assets.GetShaderDefinition(Madam::UUID("editor://shaders/Custom.frag"));
		if (!Check(updatedShader.IsValid() && updatedShader.textures.size() == 3 &&
			updatedShader.textures[2].name == "extraMap",
			"Cached shader inputs were not refreshed after a source edit")) return 1;
		auto createdShader = assets.CreateShader(customProject / "Assets" / "New Shader.frag");
		if (!Check(createdShader && std::filesystem::is_regular_file(customProject / "Assets" / "New Shader.vert"),
			"Shader asset and vertex stage were not created")) return 1;
		std::string shaderDiagnostic;
		if (!Check(assets.CompileShader(createdShader->handle, &shaderDiagnostic) &&
			std::filesystem::is_regular_file(customProject / "Library" / "Shaders" /
				((std::string)createdShader->handle + ".frag.spv")),
			"Project shader did not compile")) return 1;
		auto shaderMaterial = assets.CreateMaterial(customProject / "Assets" / "ShaderMaterial.mat");
		if (!Check(shaderMaterial != nullptr, "Could not create shader material")) return 1;
		shaderMaterial->shader = createdShader->handle;
		if (!Check(assets.SaveAsset(shaderMaterial), "Could not save shader material reference")) return 1;
		std::ofstream(customProject / "Assets" / "New Shader.frag", std::ios::trunc)
			<< "#version 450\nthis is invalid GLSL\n";
		if (!Check(!assets.CompileShader(createdShader->handle, &shaderDiagnostic) &&
			!shaderDiagnostic.empty(), "Invalid shader did not report a compile error")) return 1;
		if (!Check(assets.GetRegistryLocation() == customRegistry, "Custom registry location was ignored")) return 1;
		std::ofstream(customProject / importedTexture) << "texture data";
		assets.AppendMetaData(customProject / importedTexture);
		textureHandle = assets.GetMetadata(importedTexture).uuid;
		if (!Check(textureHandle != null, "Imported texture was not registered")) return 1;
		const auto textures = assets.ListAssets(AssetType::TEXTURE);
		if (!Check(textures.size() == 2 &&
			std::any_of(textures.begin(), textures.end(), [](const AssetReference& item) { return item.editorResource; }),
			"Asset picker did not combine editor and project textures")) return 1;
		const auto meshes = assets.ListAssets(AssetType::MESH);
		if (!Check(meshes.size() == 1 && meshes.front().editorResource &&
			meshes.front().path.filename() == "EditorMesh.obj",
			"Editor mesh resources were not listed")) return 1;
		if (!Check(!std::filesystem::exists(customProject / "Resources" / "Resources.mmeta"),
			"Asset metadata was written to the wrong registry")) return 1;
	}
	{
		AssetManager assets(customProject, customRegistry);
		if (!Check(assets.GetMetadata(importedTexture).uuid == textureHandle,
			"Imported texture ID was not saved immediately")) return 1;
	}

	const auto corruptProject = testRoot / "Corrupt";
	std::filesystem::create_directories(corruptProject / "Assets");
	std::filesystem::create_directories(corruptProject / "Resources");
	const auto corruptRegistry = corruptProject / "Resources" / "Resources.mmeta";
	std::ofstream(corruptRegistry) << "m_registry: [invalid";
	{
		AssetManager assets(corruptProject);
		if (!Check(!assets.SaveMetaData(), "A corrupt registry was overwritten")) return 1;
		std::ifstream file(corruptRegistry);
		std::stringstream contents;
		contents << file.rdbuf();
		if (!Check(contents.str() == "m_registry: [invalid", "Corrupt registry content changed")) return 1;
	}

	const auto scriptProject = testRoot / "ScriptProject";
	std::filesystem::create_directories(scriptProject / "Assets");
	std::filesystem::create_directories(scriptProject / "Resources");
	std::ofstream(scriptProject / "ProjectInfo.conf")
		<< "ProjectName: ScriptProject\nProjectVersion: 1\nProjectAuthor: Test\n"
		<< "AssetPackLocation: Resources/Resources.mmeta\n";
	std::ofstream(scriptProject / "Resources" / "Resources.mmeta") << "m_registry: []\n";
	if (!Check(Project::loadProject(scriptProject), "Could not load C# test project")) return 1;
	{
		auto& assets = Project::Get().getAssetManager();
		auto script = assets.CreateCSharpScript(scriptProject / "Assets" / "Move.cs");
		if (!Check(script != nullptr, "Could not create a C# script asset")) return 1;
		std::ofstream(scriptProject / "Assets" / "Move.cs", std::ios::trunc)
			<< "using Maiden;\npublic class Move : Behaviour {\n"
			<< "public override void OnCreate() { var p = Entity.Position; p.X += 1; Entity.Position = p; }\n"
			<< "public override void OnUpdate(float dt) { if (dt < 0) throw new System.InvalidOperationException(\"Expected script failure\"); var p = Entity.Position; p.X += dt; Entity.Position = p; }\n"
			<< "public override void OnDestroy() { var p = Entity.Position; p.Y = 7; Entity.Position = p; }\n}\n";
		auto scene = CreateRef<Scene>();
		auto entity = scene->CreateEntity();
		entity.AddComponent<CCSharpScript>().script = script->handle;
		auto runtimeScene = scene->Copy();
		auto runtimeEntity = runtimeScene->GetEntity(entity.GetComponent<CUniqueIdentifier>().uuid);
		if (!Check(runtimeEntity.HasComponent<CCSharpScript>() &&
			runtimeEntity.GetComponent<CCSharpScript>().script == script->handle,
			"C# script reference was not copied to the runtime scene")) return 1;
		Scripting::ScriptEngine host;
		if (!Check(host.Start(scriptProject, runtimeScene), "C# runtime did not start")) return 1;
		if (!Check(std::abs(runtimeEntity.GetComponent<CTransform>().translation.x - 1.0f) < 0.001f,
			"C# OnCreate did not update the entity")) return 1;
		host.Update(0.5f);
		if (!Check(std::abs(runtimeEntity.GetComponent<CTransform>().translation.x - 1.5f) < 0.001f,
			"C# OnUpdate did not update the entity")) return 1;
		host.Update(-1.0f);
		const auto consoleMessages = Logger::GetConsoleMessages();
		if (!Check(std::any_of(consoleMessages.begin(), consoleMessages.end(), [](const ConsoleMessage& message) {
			return message.level == spdlog::level::err &&
				message.text.find("C# OnUpdate failed") != std::string::npos &&
				message.text.find("Expected script failure") != std::string::npos;
			}), "C# exceptions were not captured as GUI console errors")) return 1;
		host.Stop();
		if (!Check(std::abs(runtimeEntity.GetComponent<CTransform>().translation.y - 7.0f) < 0.001f &&
			entity.GetComponent<CTransform>().translation.x == 0.0f,
			"C# OnDestroy did not run")) return 1;
		if (!Check(host.Start(scriptProject, runtimeScene), "C# runtime could not reload")) return 1;
		host.Stop();
	}
	Project::clear();

	Logger::GetCoreLogger().reset();
	Logger::GetClientLogger().reset();
	Logger::GetFileLogger().reset();
	spdlog::shutdown();
	std::filesystem::current_path(previousDirectory);
	if (testRoot.parent_path() == temporary)
		std::filesystem::remove_all(testRoot);
	std::cout << "Asset system tests passed\n";
	return 0;
}
