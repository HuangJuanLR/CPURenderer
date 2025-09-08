#pragma once
#include "SDL3/SDL.h"
#include <glm.hpp>

class Graphics {
public:
    static void Line(int x1, int y1, int x2, int y2, int zoom, const SDL_Color color);
	static void Line(glm::vec2 start, glm::vec2 end, int zoom, const SDL_Color color);
	static void Circle(int xc, int yc, int radius, int zoom, const SDL_Color color);
private:
	static void DrawCirclePoint(int xc, int yc, int x, int y, int zoom, const SDL_Color color);
};
