#pragma once

#ifdef MADAM_PLATFORM_WINDOWS
	#ifdef MADAM_BUILD_DLL
		#define MADAM_API __declspec(dllexport)
	#else
		#define MADAM_API __declspec(dllimport)
	#endif
#else
	#error Iron Maiden only supports Windows
#endif