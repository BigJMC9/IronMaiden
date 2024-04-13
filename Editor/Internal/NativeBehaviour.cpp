#include "H_NativeBehaviour.hpp"

/*[Include]*/

namespace Madam {

	NativeScriptComponent TestClass() {
		//NativeScriptComponent nsc = entity.AddComponent<NativeScriptComponent>();
		NativeScriptComponent nsc;
		nsc.Bind<controller>();
		return nsc;
	}

	//Maybe use an identifier to determine which script to bind?
	NativeScriptComponent BindType(std::string type) {

		/*[Insert]*/

		NativeScriptComponent nsc;
		nsc.Bind<controller>();
		return nsc;
	}

	void Testing123() {
		std::cout << "Hello World" << std::endl;
	}
}