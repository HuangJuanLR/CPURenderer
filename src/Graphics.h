#pragma once
#include "SDL3/SDL.h"
#include <glm.hpp>

class Graphics {
public:
    static void Line(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int zoom);
	static void Line(SDL_Renderer* renderer, glm::vec2 start, glm::vec2 end, int zoom);
	static void Circle(int xc, int yc, int radius, int zoom);
	static void Grid(SDL_Renderer* renderer, const int width, const int height, const int size);
private:
	static void DrawCirclePoint(int xc, int yc, int x, int y, int zoom);
};
