#include <Maiden.hpp>

class Editor : public Madam::Application {
public:
	Editor() : Madam::Application()
	{

	}

	~Editor()
	{

	}

};

Madam::Application* Madam::CreateApplication() 
{
	return new Editor();
}