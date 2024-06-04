#pragma once

#ifdef MADAM_PLATFORMS_HPP
#error "Platforms.hpp already included"
#else
#define MADAM_PLATFORMS_HPP
#endif


#ifdef MADAM_PLATFORM_WINDOWS
#include "Windows.hpp"

#elif defined(MADAM_PLATFORM_LINUX)
#include "MacOS.hpp"

#elif defined(MADAM_PLATFORM_MACOS)
#include "Linux.hpp"

#else
#error "Platform not supported"
#endif
