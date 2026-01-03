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
	VulkanStaticMesh::VulkanStaticMesh(const std::filesystem::path& file_path) : m_device{ Rendering::Renderer::GetDevice() }
	{
		mesh_primative = MeshPrimatives::None;
		m_filepath = file_path;

		if (file_path.extension().string() == ".obj") {
			LoadOBJ();
		}
		else if (file_path.extension().string() == ".fbx") {
			LoadFBX();
		}
		else {
			MADAM_ERROR("Model file type not supported");
		}
	}

	VulkanStaticMesh::VulkanStaticMesh(const MeshPrimatives primative) : m_device{ Rendering::Renderer::GetDevice() }
	{
		mesh_primative = primative;
		LoadPrimative(primative);
	}

	VulkanStaticMesh::~VulkanStaticMesh()
	{
		m_vertex_buffer = nullptr;
		m_index_buffer = nullptr;
	}

	void VulkanStaticMesh::LoadOBJ() 
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, m_filepath.string().c_str())) {
			MADAM_CORE_ERROR("Could not load: {0}", m_filepath.string().c_str());
			is_loaded = false;
			return;
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

		if (vertices.size() != 0 && indices.size() != 0)
		{
			CreateVertexBuffers(vertices);
			CreateIndexBuffers(indices);
		}

		is_loaded = true;
	}

	void VulkanStaticMesh::LoadFBX()
	{
		MADAM_CORE_NOT_IMPL("Load FBX file.");
		is_loaded = true;
	}

	void VulkanStaticMesh::LoadPrimative(const MeshPrimatives primative)
	{
		if (primative == MeshPrimatives::None)
		{
			MADAM_CORE_ERROR("Cannot load a primative that does not exist.");
		}
		m_filepath = meshPrimativeFilepaths[(uint8_t)primative];
		LoadOBJ();
	}

	void VulkanStaticMesh::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_vertex_count = static_cast<uint32_t>(vertices.size());
		MADAM_CORE_ASSERT(m_vertex_count >= 3, "Vertex count must be atleast 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertex_count;
		uint32_t vertexSize = sizeof(vertices[0]);

		Buffer stagingBuffer{
			m_device,
			vertexSize,
			m_vertex_count,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)vertices.data());

		m_vertex_buffer = std::make_unique<Buffer>(
			m_device,
			vertexSize,
			m_vertex_count,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		m_device.copyBuffer(stagingBuffer.GetBuffer(), m_vertex_buffer->GetBuffer(), bufferSize);
	}

	void VulkanStaticMesh::CreateIndexBuffers(const std::vector<uint32_t>& indices)
	{
		m_index_count = static_cast<uint32_t>(indices.size());
		has_index_buffer = m_index_count > 0;

		if (!has_index_buffer) {
			return;
		}
		VkDeviceSize bufferSize = sizeof(indices[0]) * m_index_count;
		uint32_t indexSize = sizeof(indices[0]);

		Buffer stagingBuffer
		{
			m_device,
			indexSize,
			m_index_count,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)indices.data());

		m_index_buffer = std::make_unique<Buffer>(m_device, indexSize, m_index_count, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_device.copyBuffer(stagingBuffer.GetBuffer(), m_index_buffer->GetBuffer(), bufferSize);
	}

	void VulkanStaticMesh::Draw(void* commandBuffer) {

		VkCommandBuffer cmdBuffer = static_cast<VkCommandBuffer>(commandBuffer);

		if (has_index_buffer) {
			vkCmdDrawIndexed(cmdBuffer, m_index_count, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(cmdBuffer, m_vertex_count, 1, 0, 0);
		}
	}

	//Move to render system and change to get the VertexBuffer
	void VulkanStaticMesh::Bind(void* commandBuffer)
	{
		if (!is_loaded)
		{
			return;
		}
		VkCommandBuffer cmdBuffer = static_cast<VkCommandBuffer>(commandBuffer);

		VkBuffer buffers[] = { m_vertex_buffer->GetBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);

		if (has_index_buffer) {
			vkCmdBindIndexBuffer(cmdBuffer, m_index_buffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
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