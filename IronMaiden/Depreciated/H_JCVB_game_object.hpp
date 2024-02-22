#pragma once

#include "H_JCVB_base.hpp"
#include "H_JCVB_model.hpp"
#include "H_JCVB_texture.hpp"
#include "H_JCVB_component.hpp"
#include "H_JCVB_mesh_renderer.hpp"

//libs
#include <glm/gtc/matrix_transform.hpp>
//std
#include <memory>
#include <unordered_map>
#include <iostream>

namespace Digestion {
	class JcvbComponent;

	struct TransformComponent {
		glm::vec3 translation{}; // (position offset)
		glm::vec3 scale{ 1.f, 1.f, 1.f};
		glm::vec3 rotation{};

		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4();

		glm::mat3 normalMatrix();
	};

	struct PointLightComponent {
		float lightIntensity = 1.0f;
	};

	class GameObject {
	public:
		static GameObject createGameObject(const std::string& jcvbName = "default") {
			static id_t currentId = 0;
			std::cout << "Creating gameobject with name: " << jcvbName << std::endl;
			return GameObject{ currentId++, jcvbName };
		}

		static GameObject makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

		GameObject(const GameObject &) = delete;
		GameObject& operator=(const GameObject&) = delete;
		GameObject(GameObject&&) = default;
		GameObject& operator=(GameObject&&) = default;

		id_t getId() { return id; }

		std::string getName() { return name; }

		void addComponent(std::unique_ptr<JcvbComponent> component);

		template<typename T>
		T* getComponent() const {
			static_assert(std::is_base_of<JcvbComponent, T>::value, "T must be derived from JcvbComponent");

			for (const auto& component : components) {
				// Use dynamic_cast on the raw pointer inside the unique_ptr
				if (auto derivedComponent = dynamic_cast<T*>(component.get())) {
					return derivedComponent;
				}
			}

			return nullptr;
		}
		void start();
		void update();
		void render();

		glm::vec3 color{};
		TransformComponent transform{};

		std::vector<std::unique_ptr<JcvbComponent>> components;
		std::string name = "default";
		//Optional pointer components
		//std::shared_ptr<JcvbModel> model{};
		//std::shared_ptr<JcvbTexture> diffuseMap = nullptr;
		//std::shared_ptr<JcvbTexture> normalMap = nullptr;
		//std::shared_ptr<JcvbTexture> ambientOcclusionMap = nullptr;
		//std::shared_ptr<JcvbTexture> glossMap = nullptr;
		std::unique_ptr<PointLightComponent> pointLight = nullptr;

	private:
		GameObject(id_t objId, const std::string& jcvbName) : id{ objId }, name{ jcvbName } { std::cout << "Still creating gameobject with name: " << jcvbName << std::endl; }
		id_t id;
	};
}