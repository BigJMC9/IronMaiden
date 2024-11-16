#pragma once

#include "maidenpch.hpp"

#define INCLUDE_LOGGER
#define INCLUDE_GLFW
#include "../Utils/H_Utils.h"

namespace Madam {

	enum CursorState : uint32_t
	{
		NORMAL = GLFW_CURSOR_NORMAL,
		HIDDEN = GLFW_CURSOR_HIDDEN,
		DISABLED = GLFW_CURSOR_DISABLED
	};

	enum CursorType : uint32_t
	{
		ARROW = GLFW_ARROW_CURSOR,
		HAND = GLFW_HAND_CURSOR,
		CROSSHAIR = GLFW_CROSSHAIR_CURSOR,
		TEXT = GLFW_IBEAM_CURSOR,
	};

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

		void init(uint32_t w, uint32_t h, std::string name);
		void deinit();

		bool shouldClose() { return glfwWindowShouldClose(window); }
		void quit() { glfwSetWindowShouldClose(window, GLFW_TRUE); }
		VkExtent2D getExtent() { return { data.width, data.height }; }
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }
		GLFWwindow* getGLFWwindow() const {
			return window;
		}

		int getWidth() const { return data.width; }
		int getHeight() const { return data.height; }
		std::string getName() const { return data.windowName; }

		void SetCursorState(CursorState cursorState);
		void SetCursorIcon(CursorType cursorType);
		void PopCursorPosition();

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		double cursorX, cursorY;

	private:
		static void framebufferResizeCallback(GLFWwindow* _window, int width, int height);
		void initWindow();
			
		bool framebufferResized = false;

		WindowData data;
		GLFWwindow* window;
		CursorType currentCursorType = CursorType::ARROW;

		bool isRunning = false;
	};
}