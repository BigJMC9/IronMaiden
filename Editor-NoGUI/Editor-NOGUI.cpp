// std
#include <windows.h>
#include <iostream>

#include <windows.h>
#include <iostream>
#include <string>

int main() {
    HANDLE hPipe;
    char buffer[1024];
    DWORD bytesRead, bytesWritten;

    // Connect to the pipe
    do {
        hPipe = CreateFile(L"\\\\.\\pipe\\MyPipe",
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);
        if (hPipe == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open pipe. Error code: " << GetLastError() << std::endl;
        }
    } while (hPipe == INVALID_HANDLE_VALUE);

    std::cout << "Connected to the pipe." << std::endl;

    while (true) {
        if (!ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
            std::cerr << "Failed to read from pipe. Error code: " << GetLastError() << std::endl;
        }

        buffer[bytesRead] = '\0';
        std::cout << buffer << std::endl;

        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);
        const char* message = input.c_str();
        if (!WriteFile(hPipe, message, strlen(message), &bytesWritten, NULL)) {
            std::cerr << "Failed to write to pipe. Error code: " << GetLastError() << std::endl;
        }
    }
    

    

    // Now the pipe is connected and ready for communication

    // Close the pipe handle
    CloseHandle(hPipe);

    std::cout << "Press Enter to exit...";
    std::cin.get();
    return EXIT_SUCCESS;
}