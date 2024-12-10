#pragma once
#include "../Core/H_Logger.hpp"
#include "../Core/H_Window.hpp"
#include "H_Input.hpp"
namespace Madam::Events {

	class Event {
		UUID uuid;
	};

	// Application Events
	
	// Window Events
	class WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(WindowData data) : windowData{ data } {}
		~WindowResizeEvent() {}
		WindowData windowData;
	};
	// Input Events
	class MouseMoveEvent : public Event {
	public:
		MouseMoveEvent(glm::vec2 pos, glm::vec2 delta, float speed) : mousePosition{ pos }, mouseDelta{ delta }, mouseSpeed{ speed } {}
		~MouseMoveEvent() {}
		glm::vec2 mousePosition{ 0.0f };
		glm::vec2 mouseDelta{ 0.0f };
		float mouseSpeed = 0.0f;
	};

	class MouseMoveRawEvent : public Event {
	public:
		MouseMoveRawEvent(double _x, double _y) : x{ _x }, y{ _y } {}
		~MouseMoveRawEvent() {}
		double x, y;
	};

	class MouseScrollEvent : public Event {
	public:
		MouseScrollEvent(double xoffset, double yoffset) : x{ xoffset }, y{ yoffset } {}
		~MouseScrollEvent() {}
		double x, y;
	};

	class KeyPressEvent : public Event {
	public:
		KeyPressEvent(KeyCode _keys) : keys{ _keys } {}
		~KeyPressEvent() {}
		KeyCode keys;
	};

	class KeyReleaseEvent : public Event {
	public:
		KeyReleaseEvent(KeyCode _keys) : keys{ _keys } {}
		~KeyReleaseEvent() {}
		KeyCode keys;
	};
	// Audio Events
	// Physics Events
	// Renderer Events
	class NextRenderPassEvent : public Event {
	public:
		NextRenderPassEvent(int index) : renderpassIndex{ index } {}
		~NextRenderPassEvent() {}
		int renderpassIndex;
	};

	// Scene Events
	class SceneChangeEvent : public Event {
	public:
		SceneChangeEvent() {}
		~SceneChangeEvent() {}
		UUID sceneID;
	};
}