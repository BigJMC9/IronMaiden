#pragma once

#include "maidenpch.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Madam {

	//Maybe change class as it may cause conflictions between class Window and Win32
	//Convert Window to struct type may stop instancing issues
	class Window {

	public:
		Window(int w, int h, std::string name);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		bool shouldClose() { return glfwWindowShouldClose(window); }
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }
		GLFWwindow* getGLFWwindow() const {
			return window;
		}

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		private:
			static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
			void initWindow();

			int width;
			int height;
			bool framebufferResized = false;

			std::string windowName;
			GLFWwindow* window;
	};
}