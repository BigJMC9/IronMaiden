#pragma once
#include "../H_Mesh.h"

namespace Madam
{
	class VulkanStaticMesh : public StaticMesh
	{
	public:
		VulkanStaticMesh(const std::filesystem::path& file_path);
		VulkanStaticMesh(const MeshPrimatives primative);
		~VulkanStaticMesh() override;

		virtual void Bind(void* command_buffer) override;
		virtual void Draw(void* command_buffer) override;

		struct Vertex
		{
			glm::vec3 position{ 0, 0, 0 };
			glm::vec3 color{ 1, 1, 1 };
			glm::vec3 normal{ 0, 0, 0 };
			glm::vec2 uv{ 0, 0 };

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

			bool operator==(const Vertex& other) const
			{
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
			}
		};

		ResourceDescriptorInfo GetDescriptorInfo() const override
		{
			return (ResourceDescriptorInfo)&m_info;
		}

	private:
		void LoadOBJ();
		void LoadFBX();
		void LoadPrimative(const MeshPrimatives primative);

		void CreateVertexBuffers(const std::vector<Vertex>& vertices);
		void CreateIndexBuffers(const std::vector<uint32_t>& indices);

		Device& m_device;
		Scope<Buffer> m_vertex_buffer = nullptr;
		uint32_t m_vertex_count = 0;
		Scope<Buffer> m_index_buffer = nullptr;
		uint32_t m_index_count = 0;

		VkDescriptorSetAllocateInfo m_info{};

		bool has_index_buffer = false;
	};
}