#pragma once
//#include <Maiden.hpp>
#include <iostream>

namespace Madam {
	class Entity;
	struct NativeScriptComponent;

	extern "C" __declspec(dllexport) NativeScriptComponent TestClass();

	extern "C" __declspec(dllexport) NativeScriptComponent BindType(std::string type);

	extern "C" __declspec(dllexport) void Testing123();

}