#pragma once
#include "SDL3/SDL.h"
#include <glm.hpp>
#include <vector>

class Graphics {
public:
    static void Line(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int zoom);
	static void Line(SDL_Renderer* renderer, glm::vec2 start, glm::vec2 end, int zoom);
	static void Circle(int xc, int yc, int radius, int zoom);
	static void Grid(SDL_Renderer* renderer, const int width, const int height, const int size);

	static void Triangle(SDL_Renderer* renderer, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2);
	static void Triangle(SDL_Renderer* renderer, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, const int& width, const int& height, std::vector<uint8_t>& zbuffer);
	static double SignedTriangleArea(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2);
private:
	static void DrawCirclePoint(int xc, int yc, int x, int y, int zoom);
};
