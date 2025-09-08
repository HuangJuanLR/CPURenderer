#include <math.h>
#include "Graphics.h"

void Graphics::Line(int x1, int y1, int x2, int y2, int zoom, const SDL_Color color) {
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
				// steep? DrawPixel(y + w, i + z, color) : DrawPixel(i + z, y + w, color);
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

void Graphics::Line(glm::vec2 start, glm::vec2 end, int zoom, const SDL_Color color)
{
	Line(start.x, start.y, end.x, end.y, zoom, color);
}

void Graphics::Circle(int xc, int yc, int radius, int zoom, const SDL_Color color)
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

		DrawCirclePoint(xc, yc, x, y, zoom, color);
	}
}

void Graphics::DrawCirclePoint(int xc, int yc, int x, int y, int zoom, const SDL_Color color)
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
