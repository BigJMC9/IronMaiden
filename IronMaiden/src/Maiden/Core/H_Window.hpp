#pragma once

#include "maidenpch.hpp"
#include "Main/Core.hpp"
#include "H_Logger.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Madam {

	

	struct WindowData {
		uint32_t width;
		uint32_t height;
		std::string windowName;
	};

	//Maybe change class as it may cause conflictions between class Window and Win32
	//Convert Window to static or struct type may stop instancing issues
	class MADAM_API Window {

	public:
		Window();
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		void StartUp(uint32_t w, uint32_t h, std::string name);
		void ShutDown();

		bool shouldClose() { return glfwWindowShouldClose(window); }
		VkExtent2D getExtent() { return { data.width, data.height }; }
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }
		GLFWwindow* getGLFWwindow() const {
			return window;
		}

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		static void framebufferResizeCallback(GLFWwindow* _window, int width, int height);
		void initWindow();
			
		bool framebufferResized = false;

		WindowData data;
		GLFWwindow* window;

		bool isRunning = false;
	};
}