#include "maidenpch.hpp"
#include "H_ProcessHandler.hpp"
#include "H_Logger.hpp"

namespace Madam {
	namespace App {
        PipeHandler::PipeHandler() : p1(NULL), p2(NULL) {}

        PipeHandler::~PipeHandler() {
            isRunning = false;
            if (p1 != NULL)
                CloseHandle(p1);
            if (p2 != NULL)
                CloseHandle(p2);
        }

        bool PipeHandler::CreatePipe() {
            if (!CreateWrite()) {
                return false;
            }
            if (!CreateRead()) {
                return false;
            }
            return true;
        }

        bool PipeHandler::CreateWrite() {
            SECURITY_ATTRIBUTES saAttr;
            saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
            saAttr.bInheritHandle = TRUE;
            saAttr.lpSecurityDescriptor = NULL;

            p1 = CreateNamedPipe(L"\\\\.\\pipe\\ToEditorPipe",
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                1,
                1024,
                1024,
                NMPWAIT_USE_DEFAULT_WAIT,
                NULL);

            if (p1 == INVALID_HANDLE_VALUE) {
                return false;
            }
            else {
                while (!ConnectNamedPipe(p1, NULL)) {
                    MADAM_CORE_ERROR("Failed to wait for client connection. Error code: {0}", GetLastError());
                }
                MADAM_CORE_INFO("Client connected to the pipe.");
                Write("Connection Successful! \n");
                Write("[!n!]");
                return true;
            }
        }

        //Update to make sure if connection fails it doesn't hang in infinite loop
        bool PipeHandler::CreateRead() {
            do {
                p2 = CreateFile(L"\\\\.\\pipe\\ToEnginePipe",
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);
                if (p2 == INVALID_HANDLE_VALUE) {
                    MADAM_CORE_ERROR("Failed to open pipe. Error code: {0}", GetLastError());
                }
            } while (p2 == INVALID_HANDLE_VALUE);

            CHAR chBuf[1024];
            DWORD dwRead;
            while (!ReadFile(p2, chBuf, sizeof(chBuf), &dwRead, NULL)) {
                MADAM_CORE_ERROR("Failed to read from pipe. Error code: {0}", GetLastError());
            }
            chBuf[dwRead] = '\0';
            MADAM_CORE_INFO("Buffer: {0}", chBuf);
            return true;
        }

        void PipeHandler::StartAsyncRead() {
            // Start a new thread for asynchronous reading
            MADAM_CORE_INFO("Starting async read.");
            isRunning = true;
            std::thread(&PipeHandler::ReadAsync, this).detach();
        }

        bool PipeHandler::Write(const std::string& message) {
            DWORD dwWritten;
            MADAM_CORE_INFO("Write: {0}", message);
            BOOL bSuccess = WriteFile(p1, message.c_str(), strlen(message.c_str()), &dwWritten, NULL);
            if (!bSuccess) {
                DWORD dwError = GetLastError();
                if (dwError == ERROR_PIPE_BUSY) {
                    // Pipe is full, handle accordingly or retry after waiting
                    MADAM_CORE_ERROR("Pipe is full. Unable to write.");
                }
                else {
                    MADAM_CORE_ERROR("Failed to write to pipe. Error code: {0}", dwError);
                }
            }
            if (dwWritten != message.length()) {
                
                MADAM_CORE_ERROR("Incomplete write to pipe. Expected {0} bytes, wrote {1} bytes.", message.length(), dwWritten);
                return false;
            }
            return bSuccess && (dwWritten == message.length());
        }

        void PipeHandler::ReadAsync() {
            // Function to read asynchronously
            CHAR chBuf[1024];
            DWORD dwRead;
            while (isRunning) {
                if (ReadFile(p2, chBuf, sizeof(chBuf), &dwRead, NULL)) {
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