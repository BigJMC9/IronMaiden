#pragma once

#ifdef MADAM_PLATFORM_WINDOWS

extern Madam::Application* Madam::CreateApplication();

int main(int argc, char** argv) 
{
	Madam::Logger::Init();
	MADAM_CORE_WARN("Initialized Log!");
	int a = 5;
	MADAM_INFO("Client! Var={0}", a);

	auto app = Madam::CreateApplication();
	app->run();
	delete app;
}

#endif // MADAM_PLATFORM_WINDOWS
