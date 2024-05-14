#pragma once

#include "maidenpch.hpp"
#include "Event.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Madam {
	class MADAM_API WindowCreateEvent : public Event {
	public:
		WindowCreateEvent(int w, int h, std::string name) : width(w), height(h), windowName(name) {}

		inline int getWidth() const { return width; }
		inline int getHeight() const { return height; }
		inline std::string getTitle() const { return windowName; }

		std::string toString() const override {
			std::stringstream ss;
			ss << "WindowCreateEvent: " << width << "x" << height << " (" << windowName << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowCreate)
		EVENT_CLASS_CATEGORY(EventCategory::EventCategoryApplication)
	private:
		unsigned int width, height;
		std::string windowName;
	};
}