#include "H_NativeBehaviour.hpp"

#include "../Assets/Scripts/controller.cpp"
#include "../Assets/Scripts/test.cpp"
/*[Include]*/

namespace Madam {

	//Maybe use an identifier to determine which script to bind?
	NativeScriptComponent BindType(std::string type) {

		if (type == "controller") {
			NativeScriptComponent nsc;
			nsc.Bind<controller>();
			return nsc;
		}
		else if (type == "test") {
			NativeScriptComponent nsc;
			nsc.Bind<test>();
			return nsc;
		}
		else {
			std::cout << "No script found" << std::endl;
			return NativeScriptComponent();
		}
		/*[Insert]*/

		NativeScriptComponent nsc;
		return nsc;
	}
}
