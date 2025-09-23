#include <iostream>

#include "App.h"
#include "Log.h"

int main(int argc, char* argv[])
{
	try
	{
		CPURDR::Log::Init();
		CPURDR::App& app = CPURDR::App::GetInstance();
		int res = app.Start();

		CPURDR::App::Destroy();

		return res;
	}
	catch (const std::exception& e)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", e.what(), nullptr);
	}

	return -1;
}
