#pragma once
#include "../Core/H_Logger.hpp"
#include "../Core/H_Window.hpp"
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