#include "H_first_app.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>
//#include <stdio.h> 
//#include <tchar.h>
//#include <strsafe.h>

int main() {
	Digestion::FirstApp app{};

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}