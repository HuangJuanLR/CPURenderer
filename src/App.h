#pragma once
#include "SDL3/SDL.h"
#include <glm.hpp>

class App
{
public:
	App();
	~App();

	int Start();

private:
	void Update();
	void CleanUp() const;
public:
	bool success = true;
private:
	SDL_Window* m_Window;
	SDL_Renderer* m_Renderer;
	int m_Width, m_Height;
	int m_LogicW, m_LogicH;
	int m_TargetFPS;
	const bool* m_Keys;
};