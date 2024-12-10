#include "maidenpch.hpp"
#include "H_EventSystem.h"
#include "../Scene/Components.hpp"
#include "../Core/H_Application.hpp"

namespace Madam {
	
	Input::Input()
	{
		glfwSetCursorPosCallback(Application::Get().GetWindow().getGLFWwindow(), CursorPositionCallback);
		glfwSetScrollCallback(Application::Get().GetWindow().getGLFWwindow(), ScrollCallback);
	}
	
	bool Input::IsKeyPress(const KeyCode key) {
		return glfwGetKey(Application::Get().GetWindow().getGLFWwindow(), static_cast<int32_t>(key)) == GLFW_PRESS;
	}

	bool Input::IsKeyHeld(const KeyCode key) {
		return glfwGetKey(Application::Get().GetWindow().getGLFWwindow(), static_cast<int32_t>(key)) == GLFW_REPEAT;
	}

	bool Input::IsKeyRelease(const KeyCode key) {
		return glfwGetKey(Application::Get().GetWindow().getGLFWwindow(), static_cast<int32_t>(key)) == GLFW_RELEASE;
	}

	bool Input::IsMouseButtonPress(const MouseCode button)
	{
		return glfwGetMouseButton(Application::Get().GetWindow().getGLFWwindow(), static_cast<int32_t>(button)) == GLFW_PRESS;
	}

	bool Input::IsMouseButtonRelease(const MouseCode button)
	{
		return glfwGetMouseButton(Application::Get().GetWindow().getGLFWwindow(), static_cast<int32_t>(button)) == GLFW_RELEASE;
	}

	glm::vec2 Input::GetMousePosition()
	{
		double xpos, ypos;
		glfwGetCursorPos(Application::Get().GetWindow().getGLFWwindow(), &xpos, &ypos);
		return { (float)xpos, (float)ypos };
	}

	void Input::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
	{
		glm::vec2 pos{ (float)ypos, (float)xpos };
		glm::vec2 previousPos = Input::Get().mousePosition;
		float threshold = Input::Get().mouseThreshold;
		float distance = glm::distance(previousPos, pos);

		if (distance > threshold)
		{
			Input::Get().mousePosition = pos;
			glm::vec2 delta = previousPos - pos;
			MouseMoveEvent e{ pos, delta, distance };
			Events::EventSystem::Get().PushEvent(&e, true);
		}

		MouseMoveRawEvent e{ xpos, ypos };
		Events::EventSystem::Get().PushEvent(&e, true);
	}

	void Input::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		MouseScrollEvent e{ xoffset, yoffset };
		Events::EventSystem::Get().PushEvent(&e, true);
	}

	void Input::KeyStateChangeCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		KeyCode keyCode = (KeyCode)key;

		if (action == GLFW_PRESS)
		{
			KeyPressEvent e{ keyCode };
			Events::EventSystem::Get().PushEvent(&e, true);
		}
		else if (action == GLFW_RELEASE)
		{
			KeyReleaseEvent e{ keyCode };
			Events::EventSystem::Get().PushEvent(&e, true);
		}
	}
}