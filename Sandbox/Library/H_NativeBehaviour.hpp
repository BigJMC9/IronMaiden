#pragma once
//#include <Maiden.hpp>
#include <iostream>

namespace Madam {
	class Entity;
	struct NativeScriptComponent;

	extern "C" __declspec(dllexport) NativeScriptComponent BindType(std::string type);

}