#pragma once

#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <array>
#include <memory>
#include <unordered_map>
#include <map>
#include <random>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>
#include <functional>
#include <set>
#include <unordered_set>
#include <limits>
#include <cmath>
#include <thread>
#include <chrono>

#ifdef MADAM_PLATFORM_WINDOWS
	#include <windows.h>
	#include <direct.h>
	#define GetCurrentDir _getcwd
#endif