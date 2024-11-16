#include "maidenpch.hpp"
#include "H_Time.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Madam {

	float Time::deltaTime = 0.0f;
	float Time::frameTime = 0.0f;
	float Time::MAX_FRAME_TIME = 0.1f;
	std::chrono::steady_clock::time_point Time::startTime = std::chrono::high_resolution_clock::now();
	std::chrono::steady_clock::time_point Time::time = std::chrono::high_resolution_clock::now();


	void Time::UpdateTime() {
		auto currentFrameTime = std::chrono::high_resolution_clock::now();
		frameTime = std::chrono::duration<float, std::chrono::seconds::period>(currentFrameTime - time).count();
		time = currentFrameTime;
		deltaTime = glm::min(frameTime, MAX_FRAME_TIME);
	}

	void Time::StartTime() {
		time = std::chrono::high_resolution_clock::now();
		startTime = std::chrono::high_resolution_clock::now();
	}
}