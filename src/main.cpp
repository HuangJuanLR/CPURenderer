#include <iostream>

#include "App.h"

int main(int argc, char* argv[])
{
	try
	{
		App& app = App::GetInstance();
		int res = app.Start();

		App::Destroy();

		return res;
	}
	catch (const std::exception& e)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", e.what(), nullptr);
	}

	return -1;
}
