#include "H_ProcessHandler.hpp"

// std
#include <thread>
#include <iostream>

namespace Digestion {
	namespace App {
        PipeHandler::PipeHandler() : hPipe(NULL) {}

        PipeHandler::~PipeHandler() {
            if (hPipe != NULL)
                CloseHandle(hPipe);
        }

        bool PipeHandler::CreatePipe() {
            SECURITY_ATTRIBUTES saAttr;
            saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
            saAttr.bInheritHandle = TRUE;
            saAttr.lpSecurityDescriptor = NULL;

            hPipe = CreateNamedPipe(L"\\\\.\\pipe\\MyPipe",
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                1,
                1024,
                1024,
                NMPWAIT_USE_DEFAULT_WAIT,
                NULL);

            if (hPipe == INVALID_HANDLE_VALUE) {
                return false;
            }
            else {
                return true;
            }
        }

        bool PipeHandler::ConnectToChildProcess(const std::string& childProcessName) {
            while (!ConnectNamedPipe(hPipe, NULL)) {
                std::cerr << "Failed to wait for client connection. Error code: " << GetLastError() << std::endl;
            }
            std::cout << "Client connected to the pipe." << std::endl;
            Write("Connection Successful!");
            return true;
        }

        void PipeHandler::StartAsyncRead() {
            // Start a new thread for asynchronous reading
            std::thread(&PipeHandler::ReadAsync, this).detach();
        }

        bool PipeHandler::Write(const std::string& message) {
            DWORD dwWritten;
            BOOL bSuccess = WriteFile(hPipe, message.c_str(), strlen(message.c_str()), &dwWritten, NULL);
            return bSuccess && (dwWritten == message.length());
        }

        void PipeHandler::ReadAsync() {
            // Function to read asynchronously
            CHAR chBuf[1024];
            DWORD dwRead;
            while (true) {
                if (ReadFile(hPipe, chBuf, sizeof(chBuf), &dwRead, NULL)) {
                    chBuf[dwRead] = '\0';
                    std::string str(chBuf);
                    message = str;
                }
            }
        }

        std::string PipeHandler::Read() {
            std::lock_guard<std::mutex> lock(mtx);
            std::string _message = message;
            message = "";
            return _message;
        }
	}
}