#pragma once
#include "maidenpch.hpp"
#include "Main/Core.hpp"

namespace Madam
{
	class MADAM_API Time
	{
	public:
		Time() = default;
		~Time() {}
		static float MAX_FRAME_TIME;
		//Returns last frame's time
		static std::chrono::steady_clock::time_point GetTime() {
			return time;
		}

		static float GetTimeSinceStart() {
			return std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();
		}

		//More Accurate
		static float GetFrameTime() {
			return frameTime;
		}
		//Takes minimum between frameTime and MAX_FRAME_TIME
		static float GetDeltaTime() {
			return deltaTime; 
		}
		static void UpdateTime();
		static void StartTime();
	private:
		static std::chrono::steady_clock::time_point time;
		static std::chrono::steady_clock::time_point startTime;
		static float deltaTime;
		static float frameTime;
	};
}