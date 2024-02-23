#pragma once

#ifdef MADAM_PLATFORM_WINDOWS

extern Madam::Application* Madam::CreateApplication();

int main(int argc, char** argv) 
{
	auto app = Madam::CreateApplication();
	app->run();
	delete app;
}

#endif // MADAM_PLATFORM_WINDOWS
