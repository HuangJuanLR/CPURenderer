#include <math.h>
#include "Graphics.h"

#include <iostream>

#include "App.h"

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

void Graphics::Triangle(SDL_Renderer* renderer, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
{
	glm::vec2 bboxMin = glm::vec2(
		std::min(p0.x, std::min(p1.x, p2.x)),
		std::min(p0.y, std::min(p1.y, p2.y))
	);
	glm::vec2 bboxMax = glm::vec2(
		std::max(p0.x, std::max(p1.x, p2.x)),
		std::max(p0.y, std::max(p1.y, p2.y))
	);

	// negative = clockwise
	double area = SignedTriangleArea(p0, p1, p2);
	if (std::abs(area) < 0.01) return;

#pragma omp parallel for
	for (int x = bboxMin.x; x < bboxMax.x; x++)
	{
		for (int y = bboxMin.y; y < bboxMax.y; y++)
		{
			// If PCA, PBC, PAB and ABC are all clock/couter-clockwise
			// P is inside ABC
			glm::vec3 p = glm::vec3(x, y, 0);
			double pbc = SignedTriangleArea(p, p1, p2) / area;
			double pca = SignedTriangleArea(p, p2, p0) / area;
			double pab = SignedTriangleArea(p, p0, p1) / area;

			if (pbc < 0 || pca < 0 || pab < 0) continue;

			// SDL_Color col = {static_cast<uint8_t>(pbc * p0.z + pca * p1.z + pab * p2.z)};
			// SDL_SetRenderDrawColor(renderer, col.r, col.r, col.r, 255);
			SDL_RenderPoint(renderer, x, y);

		}
	}
}

void Graphics::Triangle(SDL_Renderer* renderer, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2,
	const int& width, const int& height, CPURDR::Texture2D_RFloat& depthBuffer)
{
	glm::vec2 bboxMin = glm::vec2(
		std::min(p0.x, std::min(p1.x, p2.x)),
		std::min(p0.y, std::min(p1.y, p2.y))
	);
	glm::vec2 bboxMax = glm::vec2(
		std::max(p0.x, std::max(p1.x, p2.x)),
		std::max(p0.y, std::max(p1.y, p2.y))
	);

	bboxMin = glm::vec2(std::max(bboxMin.x, 0.0f), std::max(bboxMin.y, 0.0f));
	bboxMax = glm::vec2(std::min(bboxMax.x, (float)width - 1), std::min(bboxMax.y, (float)height - 1));

	// negative = clockwise
	double area = SignedTriangleArea(p0, p1, p2);
	if (std::abs(area) < 0.01) return;

#pragma omp parallel for
	for (int x = bboxMin.x; x < bboxMax.x; x++)
	{
		for (int y = bboxMin.y; y < bboxMax.y; y++)
		{
			// If PCA, PBC, PAB and ABC are all clock/couter-clockwise
			// P is inside ABC
			glm::vec3 p = glm::vec3(x, y, 0);
			double pbc = SignedTriangleArea(p, p1, p2) / area;
			double pca = SignedTriangleArea(p, p2, p0) / area;
			double pab = SignedTriangleArea(p, p0, p1) / area;

			if (pbc < 0 || pca < 0 || pab < 0) continue;

			float depth = static_cast<float>(pbc * p0.z + pca * p1.z + pab * p2.z);

			if (depth < depthBuffer(x, y))
			{
				depthBuffer(x, y) = depth;

				uint8_t depthColor = static_cast<uint8_t>((1.0f - depth) * 255.0f);
				SDL_SetRenderDrawColor(renderer, depthColor, depthColor, depthColor, 255);
				SDL_RenderPoint(renderer, x, y);
			}
		}
	}
}

void Graphics::Triangle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, const int& width, const int& height,
	CPURDR::Texture2D_RFloat& depthBuffer, CPURDR::Texture2D_RGBA& colorBuffer, uint32_t color)
{
	glm::vec2 bboxMin = glm::vec2(
		std::min(p0.x, std::min(p1.x, p2.x)),
		std::min(p0.y, std::min(p1.y, p2.y))
	);
	glm::vec2 bboxMax = glm::vec2(
		std::max(p0.x, std::max(p1.x, p2.x)),
		std::max(p0.y, std::max(p1.y, p2.y))
	);

	bboxMin = glm::vec2(
		std::max(bboxMin.x, 0.0f),
		std::max(bboxMin.y, 0.0f)
	);
	bboxMax = glm::vec2(
		std::min(bboxMax.x, (float)width - 1),
		std::min(bboxMax.y, (float)height - 1)
	);

	double area = SignedTriangleArea(p0, p1, p2);
	if (std::abs(area) < 0.01) return;

#pragma omp parallel for
	for (int x = bboxMin.x; x < bboxMax.x; x++)
	{
		for (int y = bboxMin.y; y < bboxMax.y; y++)
		{
			// If PCA, PBC, PAB and ABC are all clock/couter-clockwise
			// P is inside ABC
			glm::vec3 p = glm::vec3(x, y, 0);
			double pbc = SignedTriangleArea(p, p1, p2) / area;
			double pca = SignedTriangleArea(p, p2, p0) / area;
			double pab = SignedTriangleArea(p, p0, p1) / area;

			if (pbc < 0 || pca < 0 || pab < 0) continue;

			float depth = static_cast<float>(pbc * p0.z + pca * p1.z + pab * p2.z);

			if (depth > depthBuffer(x, y))
			{
				depthBuffer(x, y) = depth;

				uint8_t depthColor = static_cast<uint8_t>(depth * 255.0f);
				uint32_t finalColor = (depthColor << 24) | (depthColor << 16) | (depthColor << 8) | 255;
				colorBuffer(x, y) = color;
			}
		}
	}
}

double Graphics::SignedTriangleArea(const glm::vec3 p0, const glm::vec3 p1, const glm::vec3 p2)
{
	// clockwise -> -
	// counter-clockwise -> +
	return 0.5 * ((p0.y - p2.y) * (p0.x + p2.x) + (p2.y - p1.y) * (p2.x + p1.x) + (p1.y - p0.y) * (p1.x + p0.x));
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
