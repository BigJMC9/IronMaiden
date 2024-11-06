#pragma once
#include "../H_Mesh.h"

namespace Madam
{
	class VulkanStaticMesh : public StaticMesh
	{
	public:
		VulkanStaticMesh(const std::filesystem::path& filepath);
		VulkanStaticMesh(const MeshPrimatives primative);
		~VulkanStaticMesh() override;

		virtual void bind(void* commandBuffer) override;
		virtual void draw(void* commandBuffer) override;

		struct Vertex {
			glm::vec3 position{0, 0, 0};
			glm::vec3 color{1, 1, 1};
			glm::vec3 normal{0, 0, 0};
			glm::vec2 uv{0, 0};

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

			bool operator==(const Vertex& other) const {
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
			}
		};

		ResourceDescriptorInfo GetDescriptorInfo() const override
		{
			return (ResourceDescriptorInfo)&info;
		}

	private:
		void LoadOBJ();
		void LoadFBX();
		void LoadPrimative(const MeshPrimatives primative);

		void CreateVertexBuffers(const std::vector<Vertex>& vertices);
		void CreateIndexBuffers(const std::vector<uint32_t>& indices);

		Device& _device;
		Scope<Buffer> vertexBuffer = nullptr;
		uint32_t vertexCount = 0;
		Scope<Buffer> indexBuffer = nullptr;
		uint32_t indexCount = 0;

		VkDescriptorSetAllocateInfo info;

		bool hasIndexBuffer;
	};
}