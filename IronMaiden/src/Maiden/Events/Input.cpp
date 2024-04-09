#include "maidenpch.hpp"
#include "H_Input.hpp"
#include "../Scene/Components.hpp"
#include "../Core/H_Application.hpp"

namespace Madam {
	bool Input::IsKeyDown(KeyCode key) {
		return glfwGetKey(Application::Get().getWindow().getGLFWwindow(), static_cast<int>(key)) == GLFW_PRESS;
	}

	bool Input::IsKeyUp(KeyCode key) {
		return glfwGetKey(Application::Get().getWindow().getGLFWwindow(), static_cast<int>(key)) == GLFW_RELEASE;
	}
}