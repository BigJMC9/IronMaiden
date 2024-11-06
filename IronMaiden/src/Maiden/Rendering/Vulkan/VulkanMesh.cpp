#include "maidenpch.hpp"
#include "H_VulkanMesh.h"
#include "../H_Renderer.hpp"


//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <ofbx.h>

namespace std {
	template<>
	struct hash<Madam::VulkanStaticMesh::Vertex> 
	{
		size_t operator()(Madam::VulkanStaticMesh::Vertex const& vertex) const 
		{
			size_t seed = 0;
			Madam::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv/*, vertex.tangent*/);
			return seed;
		}
	};

}

namespace Madam
{
	VulkanStaticMesh::VulkanStaticMesh(const std::filesystem::path& filepath) : _device{ Rendering::Renderer::GetDevice() }
	{
		_filepath = filepath;

		if (filepath.extension().string() == ".obj") {
			LoadOBJ();
		}
		else if (filepath.extension().string() == ".fbx") {
			LoadFBX();
		}
		else {
			MADAM_ERROR("Model file type not supported");
		}
	}

	VulkanStaticMesh::VulkanStaticMesh(const MeshPrimatives primative) : _device{ Rendering::Renderer::GetDevice() }
	{
		LoadPrimative(primative);
	}

	VulkanStaticMesh::~VulkanStaticMesh()
	{
		
	}

	void VulkanStaticMesh::LoadOBJ() 
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		//when loading unsuccessful engine crashes, ensure that when loading if it is unsuccessful then bail out.
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, _filepath.string().c_str())) {
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
					vertex.position = 
					{
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2],
					};

					//rgb
					vertex.color = 
					{
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2],
					};
				}

				if (index.normal_index >= 0) {
					vertex.normal = 
					{
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2],
					};
				}

				if (index.texcoord_index >= 0) {
					vertex.uv = 
					{
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

		CreateVertexBuffers(vertices);
		CreateIndexBuffers(indices);
	}

	void VulkanStaticMesh::LoadFBX()
	{
		MADAM_CORE_NOT_IMPL("Load FBX file.");
	}

	void VulkanStaticMesh::LoadPrimative(const MeshPrimatives primative)
	{
		_filepath = meshPrimativeFilepaths[(uint32_t)primative];
		LoadOBJ();
	}

	void VulkanStaticMesh::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		MADAM_CORE_ASSERT(vertexCount >= 3, "Vertex count must be atleast 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		Buffer stagingBuffer{
			_device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		vertexBuffer = std::make_unique<Buffer>(
			_device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		_device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	void VulkanStaticMesh::CreateIndexBuffers(const std::vector<uint32_t>& indices)
	{
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) {
			return;
		}
		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		Buffer stagingBuffer
		{
			_device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		indexBuffer = std::make_unique<Buffer>(_device, indexSize, indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		_device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
	}

	void VulkanStaticMesh::draw(void* commandBuffer) {

		VkCommandBuffer cmdBuffer = static_cast<VkCommandBuffer>(commandBuffer);

		if (hasIndexBuffer) {
			vkCmdDrawIndexed(cmdBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(cmdBuffer, vertexCount, 1, 0, 0);
		}
	}

	//Move to render system and change to get the VertexBuffer
	void VulkanStaticMesh::bind(void* commandBuffer) 
	{

		VkCommandBuffer cmdBuffer = static_cast<VkCommandBuffer>(commandBuffer);

		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(cmdBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	std::vector<VkVertexInputBindingDescription> VulkanStaticMesh::Vertex::GetBindingDescriptions() 
	{
		std::vector<VkVertexInputBindingDescription> bindDescriptions(1);
		bindDescriptions[0].binding = 0;
		bindDescriptions[0].stride = sizeof(Vertex);
		bindDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> VulkanStaticMesh::Vertex::GetAttributeDescriptions() 
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });
		return attributeDescriptions;
	}
}