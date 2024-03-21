#include "maidenpch.hpp"
#include "H_Input.hpp"
#include "../Scene/Components.hpp"

namespace Madam {

	void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, Entity& gameObject) {
		if (!isHandling) {
			glm::vec3 rotate{ 0 };
			if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
			if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
			if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
			if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

			if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
				gameObject.GetComponent<Transform>().rotation += lookSpeed * dt * glm::normalize(rotate);
			}

			gameObject.GetComponent<Transform>().rotation.x = glm::clamp(gameObject.GetComponent<Transform>().rotation.x, -1.5f, 1.5f);
			gameObject.GetComponent<Transform>().rotation.y = glm::mod(gameObject.GetComponent<Transform>().rotation.y, glm::two_pi<float>());

			float yaw = gameObject.GetComponent<Transform>().rotation.y;
			const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
			const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
			const glm::vec3 upDir{ 0.f, -1.f, 0.f };

			glm::vec3 moveDir{ 0.f };
			if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
			if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
			if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
			if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
			if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
			if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

			if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
				gameObject.GetComponent<Transform>().translation += moveSpeed * dt * glm::normalize(moveDir);
			}
		}
		isHandling = false;
	}

	/*void KeyboardMovementController::handleCommands(GLFWwindow* window, SceneSerializer* scene) {
		if (!isHandling) {
			if (glfwGetKey(window, keys.leftControl) == GLFW_PRESS && glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) {
				isHandling = true;
				std::cout << "Saving Scene";
				scene->Serialize("scenes/test.scene");
			}
		}
	}*/
}