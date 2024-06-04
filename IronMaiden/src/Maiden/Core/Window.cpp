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
		//glfwDestroyWindow(window);
		//glfwTerminate();
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
		/*HWND hwnd = glfwGetWin32Window(window);

		// check state of NCRendering
		BOOL isNCRenderingEnabled{ FALSE };
		auto hr = DwmGetWindowAttribute(hwnd, DWMWA_NCRENDERING_ENABLED, &isNCRenderingEnabled, sizeof(isNCRenderingEnabled));
		if (hr != S_OK) {
			MADAM_CORE_ERROR("Failed to get DWMWA_NCRENDERING_ENABLED attribute: {0}", hr);
		}
		else {
			MADAM_CORE_INFO("Successfully got DWMWA_NCRENDERING_ENABLED attribute: {0}", hr);
		}
		RECT extendedFrameBounds{ 0,0,0,0 };
		hr = DwmGetWindowAttribute(hwnd,
			DWMWA_EXTENDED_FRAME_BOUNDS,
			&extendedFrameBounds,
			sizeof(extendedFrameBounds));
		if (hr != S_OK) {
			MADAM_CORE_ERROR("Failed to get DWMWA_EXTENDED_FRAME_BOUNDS attribute: {0}", hr);
		}
		else {
			MADAM_CORE_INFO("Successfully got DWMWA_EXTENDED_FRAME_BOUNDS attribute: {0}", hr);
		}
		COLORREF colour = 0xC32E4E40;
		DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &colour, sizeof(colour));*/
		//SetWindowRgn(hwnd, CreateRectRgn(0, 0, data.width, data.height), true);
		//SetWindowLongPtr(hwnd, GWL_STYLE, WS_EX_TRANSPARENT);
		//SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, data.width, data.height, SWP_FRAMECHANGED);
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
}