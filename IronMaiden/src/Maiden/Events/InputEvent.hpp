#pragma once

#include "maidenpch.hpp"
#include "Event.hpp"

namespace Madam {

	class MADAM_API KeyEvent : public Event {
	public:
		inline int getKeyCode() const { return keyCode; }

		EVENT_CLASS_CATEGORY(EventCategory::EventCategoryKeyboard | EventCategory::EventCategoryInput)
	protected:
		KeyEvent(int keycode) : keyCode(keycode) {}

		int keyCode;
	};

	class MADAM_API KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(int keycode, int repeatCount) : KeyEvent(keycode), repeatCount(repeatCount) {}

		inline int getRepeatCount() const { return repeatCount; }

		std::string toString() const override {
			std::stringstream ss;
			ss << "KeyPressedEvent: " << keyCode << " (" << repeatCount << " repeats)";
			return ss.str();
		}

		static EventType getStaticType() { return EventType::KeyPressed; }
		virtual EventType getEventType() const override { return getStaticType(); }
		virtual const char* getName() const override { return "KeyPressed"; }

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int repeatCount;
	};

	class MADAM_API KeyReleasedEvent : public KeyEvent {
	public:
		KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

		std::string toString() const override {
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << keyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)

	};

	class MADAM_API MouseMovedEvent : public Event {
	public:
		MouseMovedEvent(float x, float y) : mouseX(x), mouseY(y) {}

		inline float getX() const { return mouseX; }
		inline float getY() const { return mouseY; }

		std::string toString() const override {
			std::stringstream ss;
			ss << "MouseMovedEvent: " << mouseX << ", " << mouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategory::EventCategoryMouse | EventCategory::EventCategoryInput)

	private:
		float mouseX, mouseY;
	};

	class MADAM_API MouseScrolledEvent : public Event {
	public:
		MouseScrolledEvent(float xOffset, float yOffset) : xOffset(xOffset), yOffset(yOffset) {}

		inline float getXOffset() const { return xOffset; }
		inline float getYOffset() const { return yOffset; }

		std::string toString() const override {
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << xOffset << ", " << yOffset;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategory::EventCategoryMouse | EventCategory::EventCategoryInput)
	private:
		float xOffset, yOffset;
	};

	class MADAM_API MouseButtonEvent : public Event {
	public:
		inline int getMouseButton() const { return button; }

		EVENT_CLASS_CATEGORY(EventCategory::EventCategoryMouseButton | EventCategory::EventCategoryInput)
	protected:
		MouseButtonEvent(int button) : button(button) {}

		int button;
	};

	class MADAM_API MouseButtonPressedEvent : public MouseButtonEvent {
	public:
		MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}

		std::string toString() const override {
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};
}
