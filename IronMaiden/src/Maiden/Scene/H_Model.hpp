#pragma once

#include "maidenpch.hpp"
#include "../Rendering/H_Buffer.hpp"
#include "../Rendering/Vulkan/H_VulkanDevice.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Madam {
	class Model {
	public:
		
		struct Vertex {
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};
			//glm::vec3 tangent{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex &other) const {
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv  /* && tangent == other.tangent*/;
			}
		};

		struct Mesh {
			Mesh(std::vector<Vertex> vertices,std::vector<uint32_t> indices) : vertices(vertices), indices(indices){

			}
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
		};

		struct Builder {
			std::vector<Mesh> meshes{};
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
			std::string fileStr;

			void loadModel(const std::string& rawFilePath);
		private:
			void loadObjModel(const std::string& rawFilePath);
			void loadFbxModel(const std::string& rawFilePath);
		};

		Model(Device &device, const Model::Builder &builder);
		~Model();

		Model(const Model&) = delete;
		Model &operator=(const Model&) = delete;

		static Scope<Model> createModelFromFile(Device& device, const std::string& rawFilePath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

		std::string getFile() {
			return file;
		}
	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);


		Device& device;

		Scope<Buffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer = false;
		Scope<Buffer> indexBuffer;
		uint32_t indexCount;

		std::string file;
	};
}