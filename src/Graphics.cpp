#include <math.h>
#include "Graphics.h"

void Graphics::Line(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int zoom)
{
	int dx = abs(x2 - x1);
	int sx = x1 < x2? zoom : -zoom;
	int dy = -abs(y2 - y1);
	int sy = y1 < y2? zoom : -zoom;
	int error = dx + dy;

	while (true)
	{
		for (int z = -zoom; z < zoom; z++)
		{
			for (int w = -zoom; w < zoom; w++)
			{
				SDL_RenderPoint(renderer, x1 + z, y1 + w);
			}
		}
		int e2 = 2 * error;
		if (e2 >= dy) // x2 >= x1
		{
			if (x1 == x2) break;
			error += dy;
			x1 = x1 + sx;
		}
		if (e2 <= dx) // y2 >= y1
		{
			if (y1 == y2) break;
			error += dx;
			y1 = y1 + sy;
		}
	}
}

void Graphics::Line(SDL_Renderer* renderer, glm::vec2 start, glm::vec2 end, int zoom)
{
	Line(renderer, start.x, start.y, end.x, end.y, zoom);
}

void Graphics::Circle(int xc, int yc, int radius, int zoom)
{
	int d = 3 - 2 * radius;
	int x = 0, y = radius;

	while (x <= y)
	{
		if (d < 0)
		{
			d += 4 * x + 6;
		}
		else
		{
			d += 4 * (x - y) + 10;
			y--;
		}
		x++;

		DrawCirclePoint(xc, yc, x, y, zoom);
	}
}

void Graphics::Grid(SDL_Renderer* renderer, const int width, const int height, const int size)
{
	int cols = std::ceil(width / size) - 1;
	int rows = std::ceil(height / size) - 1;

	for (int col = 1; col <= cols; col++)
	{
		for (int i = 0; i < height; i++)
		{
			SDL_RenderPoint(renderer, col * size, i);
		}
	}
	for (int row = 1; row <= rows; row++)
	{
		for (int i = 0; i < width; i++)
		{
			SDL_RenderPoint(renderer, i, row * size);
		}
	}
}

void Graphics::DrawCirclePoint(int xc, int yc, int x, int y, int zoom)
{
	for (int i = -zoom; i < zoom; i++)
	{
		for (int j = -zoom; j < zoom; j++)
		{
			// DrawPixel(xc + x + i, yc + y + j, color);
			// DrawPixel(xc - x + i, yc + y + j, color);
			// DrawPixel(xc + x + i, yc - y + j, color);
			// DrawPixel(xc - x + i, yc - y + j, color);
			// DrawPixel(xc + y + i, yc + x + j, color);
			// DrawPixel(xc - y + i, yc + x + j, color);
			// DrawPixel(xc + y + i, yc - x + j, color);
			// DrawPixel(xc - y + i, yc - x + j, color);
		}
	}
}
