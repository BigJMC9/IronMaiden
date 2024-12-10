#ifndef MADAM_PLATFORMS_HPP
#error "Cannot include platform specific headers directly. You must include H_Platforms.hpp instead."
#else
#pragma once
//Windows
#include <windows.h>
#include <commdlg.h>
#include <locale>
#include <filesystem>
#include <fstream>

//Engine Specific
#include "../Core/H_Logger.hpp"

typedef DWORD PlatformFlags;
typedef LPCWSTR NPSString; //Non-Platform-Specific String
//Windows specific
namespace Madam {
	namespace Platform
	{
		static std::string ConvertWideToUtf8(const wchar_t* wideString) {
			if (wideString == nullptr) {
				return std::string();
			}

			int size = WideCharToMultiByte(CP_UTF8, 0, wideString, -1, NULL, 0, NULL, NULL);
			if (size == 0) {
				return std::string();
			}

			std::string utf8String(size, 0);
			WideCharToMultiByte(CP_UTF8, 0, wideString, -1, &utf8String[0], size, NULL, NULL);

			utf8String.resize(size - 1);

			return utf8String;
		}

		static std::wstring ConvertUtf8ToWide(const std::string& string) {
			if (string.empty()) {
				return std::wstring();
			}

			int size = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), (int)string.size(), NULL, 0);
			std::wstring wideStr(size, 0);
			MultiByteToWideChar(CP_UTF8, 0, string.c_str(), (int)string.size(), &wideStr[0], size);
			return wideStr;
		}

		static bool isChildOf(const std::filesystem::path& parent, const std::filesystem::path& child) {
			try {
				std::filesystem::path relativePath = std::filesystem::relative(child, parent);
				return !relativePath.empty() && relativePath.has_root_path() == false;
			}
			catch (const std::filesystem::filesystem_error& e) {
				MADAM_CORE_ERROR("isChildOf Filesystem error: {0} ", e.what());
				return false;
			}
		}

		static int ShowMessageBox(std::string title, std::string message, PlatformFlags flags = MB_OK, bool blockInput = true)
		{
			std::wstring _title = ConvertUtf8ToWide(title);
			std::wstring _message = ConvertUtf8ToWide(title);
			HWND hWnd = NULL;
			if (blockInput)
			{
				hWnd = GetConsoleWindow();
			}
			return MessageBox(hWnd, _title.c_str(), _message.c_str(), flags);
		}

		static int ShowMessageBox(std::wstring title, std::wstring message, PlatformFlags flags = MB_OK, bool blockInput = true)
		{
			HWND hWnd = NULL;
			if (blockInput)
			{
				hWnd = GetConsoleWindow();
			}
			return MessageBox(hWnd, title.c_str(), message.c_str(), flags);
		}

		//Open File dialog
		static bool OpenFileDialog(std::filesystem::path& filePath, PlatformFlags flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY) {
			std::filesystem::path workingDirectory = std::filesystem::current_path();
			MADAM_CORE_WARN("File extension is not specified! Not specifying file extension is not recommended!");
			MADAM_CORE_WARN("File filter is not specified! Not specifying file filter is not recommended!");
			WCHAR _filePath[MAX_PATH];
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = GetConsoleWindow();
			ofn.lpstrFilter = L"All Files\0*.*\0";
			ofn.lpstrFile = _filePath;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = flags;

			if (GetOpenFileName(&ofn)) {
				filePath = std::filesystem::path(std::wstring(_filePath));
				MADAM_QUIET_INFO("File opened: {0}", ConvertWideToUtf8(_filePath));
				std::filesystem::current_path(workingDirectory);
				return true;
			}
			else {
				std::filesystem::current_path(workingDirectory);
				return false;
			}
		}

		//Open File dialog
		static bool OpenFileDialog(std::filesystem::path& filePath, NPSString fileExtension, PlatformFlags flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY) {
			std::filesystem::path workingDirectory = std::filesystem::current_path();
			MADAM_CORE_WARN("File filter is not specified! Not specifying file filter is not recommended!");
			WCHAR _filePath[MAX_PATH];
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = GetConsoleWindow();
			ofn.lpstrFilter = L"All Files\0*.*\0";
			ofn.lpstrFile = _filePath;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrDefExt = fileExtension;
			ofn.Flags = flags;

			if (GetOpenFileName(&ofn)) {
				filePath = std::filesystem::path(std::wstring(_filePath));
				MADAM_QUIET_INFO("File opened: {0}", ConvertWideToUtf8(_filePath));
				std::filesystem::current_path(workingDirectory);
				return true;
			}
			else {
				std::filesystem::current_path(workingDirectory);
				return false;
			}
		}

		//Open File dialog
		static bool OpenFileDialog(std::filesystem::path& filePath, NPSString fileExtension, NPSString filter, PlatformFlags flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY) {
			std::filesystem::path workingDirectory = std::filesystem::current_path();
			WCHAR _filePath[MAX_PATH];
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = GetConsoleWindow();
			ofn.lpstrFilter = filter;
			ofn.lpstrFile = _filePath;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrDefExt = fileExtension;
			ofn.Flags = flags;

			if (GetOpenFileName(&ofn)) {
				filePath = std::filesystem::path(std::wstring(_filePath));
				MADAM_QUIET_INFO("File opened: {0}", ConvertWideToUtf8(_filePath));
				std::filesystem::current_path(workingDirectory);
				return true;
			}
			else {
				std::filesystem::current_path(workingDirectory);
				return false;
			}
		}

		//Save File dialog
		static bool SaveFileDialog(std::filesystem::path& filePath, PlatformFlags flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT) {
			MADAM_CORE_WARN("File extension is not specified! Not specifying file extension is not recommended!");
			MADAM_CORE_WARN("File filter is not specified! Not specifying file filter is not recommended!");
			WCHAR _filePath[MAX_PATH];
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = GetConsoleWindow();
			ofn.lpstrFilter = L"All Files\0*.*\0";
			ofn.lpstrFile = _filePath;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = flags;

			if (GetSaveFileName(&ofn)) {
				filePath = std::filesystem::path(std::wstring(_filePath));
				MADAM_QUIET_INFO("File saved: {0}", ConvertWideToUtf8(_filePath));
				return true;
			}
			else {
				return false;
			}
		}

		//Save File dialog
		static bool SaveFileDialog(std::filesystem::path& filePath, NPSString fileExtension, PlatformFlags flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT) {
			MADAM_CORE_WARN("File filter is not specified! Not specifying file filter is not recommended!");
			WCHAR _filePath[MAX_PATH];
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = GetConsoleWindow();
			ofn.lpstrFilter = L"All Files\0*.*\0";
			ofn.lpstrFile = _filePath;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrDefExt = fileExtension;
			ofn.Flags = flags;

			if (GetSaveFileName(&ofn)) {
				filePath = std::filesystem::path(std::wstring(_filePath));
				MADAM_QUIET_INFO("File saved: {0}", ConvertWideToUtf8(_filePath));
				return true;
			}
			else {
				return false;
			}
		}

		//Save File dialog
		static bool SaveFileDialog(std::filesystem::path& filePath, NPSString fileExtension, NPSString filter, PlatformFlags flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT)
		{
			WCHAR _filePath[MAX_PATH];
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = GetConsoleWindow();
			ofn.lpstrFilter = filter;
			ofn.lpstrFile = _filePath;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrDefExt = fileExtension;
			ofn.Flags = flags;

			if (GetSaveFileName(&ofn)) {
				filePath = std::filesystem::path(std::wstring(_filePath));
				MADAM_QUIET_INFO("File saved: {0}", ConvertWideToUtf8(_filePath));
				return true;
			}
			else {
				return false;
			}
		}

#ifdef CreateDirectory
#undef CreateDirectory
#endif

		static bool CreateDirectory(const std::filesystem::path path) 
		{
			try {
				if (std::filesystem::create_directory(path)) {
					MADAM_QUIET_INFO("Directory created: {0}", path.string());
					return true;
				}
				else {
					MADAM_CORE_ERROR("Failed to create directory (no specific reason provided): {0}", path.string());
					return false;
				}
			}
			catch (const std::filesystem::filesystem_error& e) {
				MADAM_CORE_ERROR("Error: {0}", e.what());
				return false;
			}
			catch (const std::exception& e) {
				MADAM_CORE_ERROR("Error: {0}", e.what());
				return false;
			}
		}

		static bool CreateDirectory(const std::filesystem::path paths[])
		{
			if (paths->empty()) {
				MADAM_CORE_ERROR("Paths array is empty!");
				return false;
			}
			for (size_t i = 0; i < (sizeof(paths)/sizeof(paths[0])); i++) {
				if (!std::filesystem::create_directory(paths[i])) {
					MADAM_CORE_ERROR("Failed to create directory: {0}", paths[i].string());
					return false;
				}
				MADAM_QUIET_INFO("Directory created: {0}", paths[i].string());
			}
			return true;
		}

		static bool CreateDirectory(const std::vector<std::filesystem::path> paths)
		{
			if (paths.empty()) {
				MADAM_CORE_ERROR("Paths vector is empty!");
				return false;
			}
			for (size_t i = 0; i < paths.size(); i++) {
				if (!std::filesystem::create_directory(paths[i])) {
					MADAM_CORE_ERROR("Failed to create directory: {0}", paths[i].string());
					return false;
				}
				MADAM_QUIET_INFO("Directory created: {0}", paths[i].string());
			}
			return true;
		}

		static bool DeleteDirectory(const std::filesystem::path path, std::string title = "Delete selected?", std::string message = "You cannot undo the delete action.")
		{
			std::wstring _title = ConvertUtf8ToWide(title);
			std::wstring _message = ConvertUtf8ToWide(message);
			int messageBoxID = ShowMessageBox(_title, _message, MB_ICONQUESTION | MB_YESNO);
			if (messageBoxID == IDYES) {
				try {
					if (std::filesystem::remove_all(path)) {
						MADAM_CORE_INFO("Directory deleted successfully.");
						return true;
					}
					else {
						MADAM_CORE_INFO("Unable to delete directory.");
						return false;
					}
				}
				catch (const std::filesystem::filesystem_error& e) {
					MADAM_CORE_ERROR("Error deleting directory: {0}", e.what());
					return false;
				}
			}
			else {
				return false;
			}
		}

		static bool DeleteDirectory(const std::filesystem::path path, std::wstring title = L"Delete selected?", std::wstring message = L"You cannot undo the delete action.")
		{
			int messageBoxID = ShowMessageBox(title, message, MB_ICONQUESTION | MB_YESNO);
			if (messageBoxID == IDYES) {
				try {
					if (std::filesystem::remove_all(path)) {
						MADAM_CORE_INFO("Directory deleted successfully.");
						return true;
					}
					else {
						MADAM_CORE_INFO("Unable to delete directory.");
						return false;
					}
				}
				catch (const std::filesystem::filesystem_error& e) {
					MADAM_CORE_ERROR("Error deleting directory: {0}", e.what());
					return false;
				}
			}
			else {
				return false;
			}
		}

		static bool DeleteDirectory(const std::filesystem::path path, int messageBoxID)
		{
			if (messageBoxID == IDYES) {
				try {
					if (std::filesystem::remove_all(path)) {
						MADAM_CORE_INFO("Directory deleted successfully.");
						return true;
					}
					else {
						MADAM_CORE_INFO("Unable to delete directory.");
						return false;
					}
				}
				catch (const std::filesystem::filesystem_error& e) {
					MADAM_CORE_ERROR("Error deleting directory: {0}", e.what());
					return false;
				}
			}
			else {
				return false;
			}
		}

		static std::ofstream OutputFile(const std::filesystem::path path) 
		{
			std::ofstream file(path);
			if (!file.is_open()) {
				std::filesystem::path parent = path.parent_path();
				if (!std::filesystem::exists(parent)) {
					MADAM_CORE_ERROR("Parent directory does not exist: {0}", parent.string());
				}
				else {
					MADAM_CORE_ERROR("Failed to open/create file: {0}", path.string());
				}
				return file;
			}
			else if (!file.good()) {
				MADAM_CORE_ERROR("File is not in good state: {0}", path.string());
				return file;
			}
			else {
				MADAM_QUIET_INFO("Output File: {0}", path.string());
				return file;
			}
		}

		static bool OutputFile(std::ofstream& file, const std::filesystem::path path) 
		{
			file = std::ofstream(path);
			if (!file.is_open()) {
				std::filesystem::path parent = path.parent_path();
				if (!std::filesystem::exists(parent)) {
					MADAM_CORE_ERROR("Parent directory does not exist: {0}", parent.string());
				}
				else {
					MADAM_CORE_ERROR("Failed to open/create file: {0}", path.string());
				}
				return false;
			}
			else if (!file.good()) {
				MADAM_CORE_ERROR("File is not in good state: {0}", path.string());
				return false;
			}
			else {
				MADAM_QUIET_INFO("Output File: {0}", path.string());
				return true;
			}
		}

		static std::ifstream OpenFile(const std::filesystem::path path) 
		{
			std::ifstream file(path);
			if (!file.is_open()) {
				std::filesystem::path parent = path.parent_path();
				if (!std::filesystem::exists(parent)) {
					MADAM_CORE_ERROR("Parent directory does not exist: {0}", parent.string());
				}
				else {
					MADAM_CORE_ERROR("Failed to open file: {0}", path.string());
				}
				return file;
			}
			else if (!file.good()) {
				MADAM_CORE_ERROR("File is not in good state: {0}", path.string());
				return file;
			}
			else {
				MADAM_QUIET_INFO("Opened File: {0}", path.string());
				return file;
			}
		}

		static bool OpenFile(std::ifstream& file, const std::filesystem::path path)
		{
			file = std::ifstream(path);
			if (!file.is_open()) {
				std::filesystem::path parent = path.parent_path();
				if (!std::filesystem::exists(parent)) {
					MADAM_CORE_ERROR("Parent directory does not exist: {0}", parent.string());
				}
				else {
					MADAM_CORE_ERROR("Failed to open file: {0}", path.string());
				}
				return false;
			}
			else if (!file.good()) {
				MADAM_CORE_ERROR("File is not in good state: {0}", path.string());
				return false;
			}
			else {
				MADAM_QUIET_INFO("Opened File: {0}", path.string());
				return true;
			}
		}
	}
}
#endif