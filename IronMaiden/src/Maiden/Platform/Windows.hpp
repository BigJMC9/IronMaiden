#ifndef MADAM_PLATFORMS_HPP
#error "Cannot include platform specific headers directly. You must include H_Platforms.hpp instead."
#else
#pragma once
//Windows
#include <windows.h>
#include <commdlg.h>
#include <locale>
#include <codecvt>
#include <filesystem>
#include <fstream>

//Engine Specific
#include "../Core/H_Logger.hpp"

//Windows specific
namespace Madam {
	namespace Platform
	{
		static std::string ConvertWideToNarrow(const wchar_t* wideString) {
			std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
			return converter.to_bytes(wideString);
		}
		//Open File dialog
		static bool OpenFileDialog(HWND hWnd, LPWSTR fileName, DWORD fileNameSize) {
			MADAM_CORE_WARN("File extension is not specified! Not specifying file extension is not recommended!");
			MADAM_CORE_WARN("File filter is not specified! Not specifying file filter is not recommended!");
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = L"All Files\0*.*\0";
			ofn.lpstrFile = fileName;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = fileNameSize;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

			if (GetOpenFileName(&ofn)) {
				MADAM_QUIET_INFO("File opened: {0}", ConvertWideToNarrow(fileName));
				return true;
			}
			else {
				return false;
			}
		}

		//Open File dialog
		static bool OpenFileDialog(HWND hWnd, LPWSTR fileName, DWORD fileNameSize, LPCWSTR fileExtension) {
			MADAM_CORE_WARN("File filter is not specified! Not specifying file filter is not recommended!");
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = L"All Files\0*.*\0";
			ofn.lpstrFile = fileName;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = fileNameSize;
			ofn.lpstrDefExt = fileExtension;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

			if (GetOpenFileName(&ofn)) {
				MADAM_QUIET_INFO("File opened: {0}", ConvertWideToNarrow(fileName));
				return true;
			}
			else {
				return false;
			}
		}

		//Open File dialog
		static bool OpenFileDialog(HWND hWnd, LPWSTR fileName, DWORD fileNameSize, LPCWSTR fileExtension, LPCWSTR filter) {
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = filter;
			ofn.lpstrFile = fileName;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = fileNameSize;
			ofn.lpstrDefExt = fileExtension;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

			if (GetOpenFileName(&ofn)) {
				MADAM_QUIET_INFO("File opened: {0}", ConvertWideToNarrow(fileName));
				return true;
			}
			else {
				return false;
			}
		}

		//Save File dialog
		static bool SaveFileDialog(HWND hWnd, LPWSTR fileName, DWORD fileNameSize) {
			MADAM_CORE_WARN("File extension is not specified! Not specifying file extension is not recommended!");
			MADAM_CORE_WARN("File filter is not specified! Not specifying file filter is not recommended!");
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = L"All Files\0*.*\0";
			ofn.lpstrFile = fileName;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = fileNameSize;
			ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;

			if (GetSaveFileName(&ofn)) {
				MADAM_QUIET_INFO("File saved: {0}", ConvertWideToNarrow(fileName));
				return true;
			}
			else {
				return false;
			}
		}

		//Save File dialog
		static bool SaveFileDialog(HWND hWnd, LPWSTR fileName, DWORD fileNameSize, LPCWSTR fileExtension) {
			MADAM_CORE_WARN("File filter is not specified! Not specifying file filter is not recommended!");
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = L"All Files\0*.*\0";
			ofn.lpstrFile = fileName;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = fileNameSize;
			ofn.lpstrDefExt = fileExtension;
			ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;

			if (GetSaveFileName(&ofn)) {
				MADAM_QUIET_INFO("File saved: {0}", ConvertWideToNarrow(fileName));
				return true;
			}
			else {
				return false;
			}
		}

		//Save File dialog
		static bool SaveFileDialog(HWND hWnd, LPWSTR fileName, DWORD fileNameSize, LPCWSTR fileExtension, LPCWSTR filter) {
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = filter;
			ofn.lpstrFile = fileName;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = fileNameSize;
			ofn.lpstrDefExt = fileExtension;
			ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;

			if (GetSaveFileName(&ofn)) {
				MADAM_QUIET_INFO("File saved: {0}", ConvertWideToNarrow(fileName));
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
			if (std::filesystem::create_directory(path)) 
			{
				MADAM_QUIET_INFO("Directory created: {0}", path.string());
				return true;
			}
			else 
			{
				MADAM_CORE_ERROR("Failed to create directory: {0}", path.string());
				return false;
			}
		}

		static bool CreateDirectory(const std::filesystem::path paths[])
		{
			if (paths->empty())
			{
				MADAM_CORE_ERROR("Paths array is empty!");
				return false;
			}
			for (size_t i = 0; i < (sizeof(paths)/sizeof(paths[0])); i++)
			{
				if (!std::filesystem::create_directory(paths[i]))
				{
					MADAM_CORE_ERROR("Failed to create directory: {0}", paths[i].string());
					return false;
				}
				MADAM_QUIET_INFO("Directory created: {0}", paths[i].string());
			}
			return true;
		}

		static bool CreateDirectory(const std::vector<std::filesystem::path> paths)
		{
			if (paths.empty())
			{
				MADAM_CORE_ERROR("Paths vector is empty!");
				return false;
			}
			for (size_t i = 0; i < paths.size(); i++)
			{
				if (!std::filesystem::create_directory(paths[i]))
				{
					MADAM_CORE_ERROR("Failed to create directory: {0}", paths[i].string());
					return false;
				}
				MADAM_QUIET_INFO("Directory created: {0}", paths[i].string());
			}
			return true;
		}

		static std::ofstream OutputFile(const std::filesystem::path path)
		{
			std::ofstream file(path);
			if (!file.is_open())
			{
				
				std::filesystem::path parent = path.parent_path();
				if (!std::filesystem::exists(parent))
				{
					MADAM_CORE_ERROR("Parent directory does not exist: {0}", parent.string());
				}
				else
				{
					MADAM_CORE_ERROR("Failed to open/create file: {0}", path.string());
				}
				return file;
			}
			else if (!file.good())
			{
				MADAM_CORE_ERROR("File is not in good state: {0}", path.string());
				return file;
			}
			else
			{
				MADAM_QUIET_INFO("Output File: {0}", path.string());
				return file;
			}
		}

		static bool OutputFile(std::ofstream& file, const std::filesystem::path path)
		{
			file = std::ofstream(path);
			if (!file.is_open())
			{

				std::filesystem::path parent = path.parent_path();
				if (!std::filesystem::exists(parent))
				{
					MADAM_CORE_ERROR("Parent directory does not exist: {0}", parent.string());
				}
				else
				{
					MADAM_CORE_ERROR("Failed to open/create file: {0}", path.string());
				}
				return false;
			}
			else if (!file.good())
			{
				MADAM_CORE_ERROR("File is not in good state: {0}", path.string());
				return false;
			}
			else
			{
				MADAM_QUIET_INFO("Output File: {0}", path.string());
				return true;
			}
		}

		static std::ifstream OpenFile(const std::filesystem::path path)
		{
			std::ifstream file(path);
			if (!file.is_open())
			{
				std::filesystem::path parent = path.parent_path();
				if (!std::filesystem::exists(parent))
				{
					MADAM_CORE_ERROR("Parent directory does not exist: {0}", parent.string());
				}
				else 
				{
					MADAM_CORE_ERROR("Failed to open file: {0}", path.string());
				}
				return file;
			}
			else if (!file.good())
			{
				MADAM_CORE_ERROR("File is not in good state: {0}", path.string());
				return file;
			}
			else
			{
				MADAM_QUIET_INFO("Opened File: {0}", path.string());
				return file;
			}
		}

		static bool OpenFile(std::ifstream& file, const std::filesystem::path path)
		{
			file = std::ifstream(path);
			if (!file.is_open())
			{
				std::filesystem::path parent = path.parent_path();
				if (!std::filesystem::exists(parent))
				{
					MADAM_CORE_ERROR("Parent directory does not exist: {0}", parent.string());
				}
				else
				{
					MADAM_CORE_ERROR("Failed to open file: {0}", path.string());
				}
				return false;
			}
			else if (!file.good())
			{
				MADAM_CORE_ERROR("File is not in good state: {0}", path.string());
				return false;
			}
			else
			{
				MADAM_QUIET_INFO("Opened File: {0}", path.string());
				return true;
			}
		}
	}
}
#endif