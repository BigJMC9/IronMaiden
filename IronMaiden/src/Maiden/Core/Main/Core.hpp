#pragma once

#ifdef MADAM_PLATFORM_WINDOWS
	#ifdef MADAM_DYNAMIC_LINK
		#ifdef MADAM_BUILD_DLL
			#define MADAM_API __declspec(dllexport)
		#else
			#define MADAM_API __declspec(dllimport)
		#endif
	#else
		#define MADAM_API
	#endif
#endif
