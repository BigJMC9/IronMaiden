// std
#include <windows.h>
#include <iostream>

#include <windows.h>
#include <iostream>
#include <string>

void read(HANDLE p1, char buffer[1024], DWORD& bytesRead) {
    while (!ReadFile(p1, buffer, sizeof(buffer), &bytesRead, NULL)) {
        std::cerr << "Failed to read from pipe. Error code: " << GetLastError() << std::endl;
    }
    buffer[bytesRead] = '\0';
}

void write(HANDLE p2, char buffer[1024], DWORD& bytesWritten, std::string input) {
    const char* message = input.c_str();
    if (!WriteFile(p2, message, strlen(message), &bytesWritten, NULL)) {
        std::cerr << "Failed to write to pipe. Error code: " << GetLastError() << std::endl;
    }
}

int main() {
    HANDLE p1; // Engine => Editor
    HANDLE p2; // Editor => Engine
    char buffer[1024];
    DWORD bytesRead, bytesWritten;

    // Connects to pipe
    do {
        p1 = CreateFile(L"\\\\.\\pipe\\ToEditorPipe",
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);
        if (p1 == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open pipe. Error code: " << GetLastError() << std::endl;
        }
    } while (p1 == INVALID_HANDLE_VALUE);

    std::cout << "Read End Pipe Established!" << std::endl;
    read(p1, buffer, bytesRead);

    p2 = CreateNamedPipe(L"\\\\.\\pipe\\ToEnginePipe",
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1,
        1024,
        1024,
        NMPWAIT_USE_DEFAULT_WAIT,
        NULL);
    if (p2 == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create pipe. Error code: " << GetLastError() << std::endl;
        return EXIT_FAILURE;
    }

    while (!ConnectNamedPipe(p2, NULL)) {
        std::cerr << "Failed to wait for Engine connection. Error code: " << GetLastError() << std::endl;
    }
    std::cout << "Create Write End Pipe!" << std::endl;
    write(p2, buffer, bytesWritten, "Connected to Editor!");

    while (true) {
        while(true) {
            read(p1, buffer, bytesRead);
            if (strstr(buffer, "[!n!]") == NULL) {
                std::cout << buffer;
            }
            else {
                std::cout << std::endl;
                break;
            }
        } //while (strstr(buffer, "[!n!]") == NULL);
        

        // Server finished writing, now handle user input
        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);
        write(p2, buffer, bytesWritten, input);
    }

    // Close the pipe handle
    CloseHandle(p1);
    CloseHandle(p2);

    std::cout << "Press Enter to exit...";
    std::cin.get();
    return EXIT_SUCCESS;
}