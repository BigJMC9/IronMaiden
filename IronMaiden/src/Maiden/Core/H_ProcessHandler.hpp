#pragma once

#include "maidenpch.hpp"
#include "Main/Core.hpp"

namespace Madam {
	namespace App {
		class MADAM_API PipeHandler {
		public:
			PipeHandler();
			~PipeHandler();

			void Shutdown() { isRunning = false; }

			bool CreatePipe();
			//bool ConnectToChildProcess(const std::string& childProcessName);
			void ReadAsync();
			void StartAsyncRead();
			bool Write(const std::string& message);
			std::string Read();

			bool isCreated = false;
			bool isConnected = false;
		private:
			bool CreateWrite();
			bool CreateRead();
			bool isRunning = false;

			HANDLE p1; // Engine => Editor
			HANDLE p2; // Editor => Engine
			std::string message;
			std::mutex mtx;
		};
	}
}