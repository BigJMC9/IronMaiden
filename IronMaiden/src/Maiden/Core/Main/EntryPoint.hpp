#pragma once

#ifdef MADAM_PLATFORM_WINDOWS

extern Madam::Application* Madam::CreateApplication();

int main(int argc, char** argv)
{
	Madam::Logger::Init();
	MADAM_CORE_WARN("Initialized Log!");

	auto app = Madam::CreateApplication();
	app->run();
	delete app;
	return 0;
}

#endif // MADAM_PLATFORM_WINDOWS
