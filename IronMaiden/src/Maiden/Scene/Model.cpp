#include "maidenpch.hpp"
#include "../Core/H_Application.hpp"
#include "H_Model.hpp"

#include "../Core/utils.hpp"

//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <ofbx.h>

namespace std {
	template<>
	struct hash<Madam::Model::Vertex> {
		size_t operator()(Madam::Model::Vertex const& vertex) const {
			size_t seed = 0;
			Madam::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv/*, vertex.tangent*/);
			return seed;
		}
	};

}

namespace Madam {

	Model::Model(Device& device, const Model::Builder& builder) : device{ device }, file{builder.fileStr} {
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	Model::~Model() {}

	std::unique_ptr<Model> Model::createModelFromFile(Device& device, const std::string& rawFilePath) {
		Builder builder{};
		builder.fileStr = Application::Get().getConfig().internals + rawFilePath;
		builder.loadModel(rawFilePath);
		return std::make_unique<Model>(device, builder);
	}

	void Model::createVertexBuffers(const std::vector<Vertex>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be atleast 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		Buffer stagingBuffer{
			device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		vertexBuffer = std::make_unique<Buffer>(
			device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	void Model::createIndexBuffers(const std::vector<uint32_t>& indices) {
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) {
			return;
		}
		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		Buffer stagingBuffer{
			device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		indexBuffer = std::make_unique<Buffer>(
			device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
	}

	//Move to render system and change to get the IndexBuffer
	void Model::draw(VkCommandBuffer commandBuffer) {
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}
	}

	//Move to render system and change to get the VertexBuffer
	void Model::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { vertexBuffer->getBuffer()};
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindDescriptions(1);
		bindDescriptions[0].binding = 0;
		bindDescriptions[0].stride = sizeof(Vertex);
		bindDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
		attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
		attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });

		return attributeDescriptions;
	}

	void Model::Builder::loadModel(const std::string& rawFilePath) {
		if (rawFilePath.substr(rawFilePath.size() - 4) == ".obj") {
			loadObjModel(rawFilePath);
		}
		else if (rawFilePath.substr(rawFilePath.size() - 4) == ".fbx") {
			loadFbxModel(rawFilePath);
		}
		else {
			throw std::runtime_error("Model file type not supported");
		}
	}

	void Model::Builder::loadObjModel(const std::string& rawFilePath) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		std::string filePath = Application::Get().getConfig().internals + rawFilePath;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};
				if (index.vertex_index >= 0) {
					//xyz
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2],
					};

					//rgb
					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2],
					};
				}

				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2],
					};
				}

				if (index.texcoord_index >= 0) {
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1],
					};
				}

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}

	void Model::Builder::loadFbxModel(const std::string& rawFilePath) {
		FILE* file = fopen(rawFilePath.c_str(), "rb");
		if (!file) {
			throw std::runtime_error("Failed to open file");
		}

		fseek(file, 0, SEEK_END);
		long fileLength = ftell(file);
		fseek(file, 0, SEEK_SET);
		auto* content = new ofbx::u8[fileLength];
		fread(content, 1, fileLength, file);

		ofbx::LoadFlags flags = 
			ofbx::LoadFlags::IGNORE_MODELS |
			ofbx::LoadFlags::IGNORE_BLEND_SHAPES |
			ofbx::LoadFlags::IGNORE_CAMERAS |
			ofbx::LoadFlags::IGNORE_LIGHTS |
	//		ofbx::LoadFlags::IGNORE_TEXTURES |
			ofbx::LoadFlags::IGNORE_SKIN |
			ofbx::LoadFlags::IGNORE_BONES |
			ofbx::LoadFlags::IGNORE_PIVOTS |
	//		ofbx::LoadFlags::IGNORE_MATERIALS |
			ofbx::LoadFlags::IGNORE_POSES |
			ofbx::LoadFlags::IGNORE_VIDEOS |
			ofbx::LoadFlags::IGNORE_LIMBS |
	//		ofbx::LoadFlags::IGNORE_MESHES |
			ofbx::LoadFlags::IGNORE_ANIMATIONS;

		/*ofbx::IScene* g_scene = ofbx::load((ofbx::u8*)content, fileLength, (ofbx::u16)flags);
		if (!g_scene) {
			throw std::runtime_error("Failed to load scene");
		}*/

		//Convert to MeshData
	}
}