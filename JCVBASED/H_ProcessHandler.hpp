#pragma once

// std
#include <windows.h>
#include <string>
#include <mutex>

namespace Digestion {
	namespace App {
		class PipeHandler {
		public:
			PipeHandler();
			~PipeHandler();

			bool CreatePipe();
			bool ConnectToChildProcess(const std::string& childProcessName);
			void ReadAsync();
			void StartAsyncRead();
			bool Write(const std::string& message);
			std::string Read();

			bool isCreated = false;
			bool isConnected = false;
		private:
			HANDLE hPipe;
			std::string message;
			std::mutex mtx;
		};
	}
}