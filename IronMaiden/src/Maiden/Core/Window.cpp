#include "maidenpch.hpp"
#include "H_Window.hpp"
#include "dwmapi.h"

namespace Madam {

	Window::Window() {
		//data.width = w;
		//data.height = h;
		//data.windowName = name;
		//initWindow();
	}

	Window::~Window() {
		if (isRunning) {
			MADAM_CORE_WARN("Window prematurally shutdown");
			deinit();
		}
	}

	void Window::init(uint32_t w, uint32_t h, std::string name) {
		data.width = w;
		data.height = h;
		data.windowName = name;
		initWindow();
		isRunning = true;
	}

	void Window::deinit() {
		glfwDestroyWindow(window);
		glfwTerminate();
		isRunning = false;
	}

	void Window::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow((int)data.width, (int)data.height, data.windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}
	}

	//update to new event system
	void Window::framebufferResizeCallback(GLFWwindow* _window, int width, int height) {
		auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(_window));
		window->framebufferResized = true;
		window->data.width = width;
		window->data.height = height;
	}

	void Window::SetCursorState(CursorState cursorState)
	{
		CursorState cursorMode = (CursorState)glfwGetInputMode(window, GLFW_CURSOR);
		if (cursorMode != cursorState)
		{
			glfwSetInputMode(window, GLFW_CURSOR, (int32_t)cursorState);
			glfwGetCursorPos(window, &cursorX, &cursorY);
		}
	}

	void Window::SetCursorIcon(CursorType cursorType)
	{
		if (currentCursorType != cursorType)
		{
			glfwSetCursor(window, glfwCreateStandardCursor((int32_t)cursorType));
			currentCursorType = cursorType;
		}
	}

	void Window::PopCursorPosition()
	{
		glfwSetCursorPos(window, cursorX, cursorY);
	}
}