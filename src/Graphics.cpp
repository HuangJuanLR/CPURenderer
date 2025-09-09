#include <math.h>
#include "Graphics.h"

void Graphics::Line(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int zoom)
{
    bool steep = std::abs(y2 - y1) > std::abs(x2 - x1);
	if (steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}
	if (x1 > x2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	int deltax = x2 - x1;
	int deltay = abs(y2 - y1);
	int error = deltax / 2;

	int ystep = y1 < y2 ? zoom : -zoom;;
	int y = y1;

	for (int i = x1; i < x2; i+=zoom)
	{
		for (int z = -zoom; z < zoom; z++)
		{
			for (int w = -zoom; w < zoom; w++)
			{
				steep? SDL_RenderPoint(renderer, y + w, i + z) : SDL_RenderPoint(renderer, i + z, y + w);
			}
		}

		error = error - deltay;
		if (error < 0)
		{
			y += ystep;
			error += deltax;
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
