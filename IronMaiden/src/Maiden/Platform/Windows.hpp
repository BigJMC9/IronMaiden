#ifndef MADAM_PLATFORMS_HPP
#error "Cannot include platform specific headers directly. You must include H_Platforms.hpp instead."
#else
#pragma once
//Windows
#include <windows.h>
#include <commdlg.h>
#include <locale>
#include <codecvt>

//Engine Specific
#include "../Core/H_Logger.hpp"

//Windows specific
namespace Madam {
	namespace Platform
	{
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
				return true;
			}
			else {
				return false;
			}
		}
	}
}
#endif