#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

#include "vec4.hpp"
#include "SDL3/SDL.h"

namespace CPURDR
{
	template<typename T>
	class Texture2D
	{
	public:
		Texture2D(size_t w, size_t h);
		Texture2D(size_t w, size_t h, const T& value);
		~Texture2D() = default;

		// Copy
		Texture2D(const Texture2D& other);
		Texture2D& operator=(const Texture2D& other);

		// Move
		Texture2D(Texture2D&& other) noexcept;
		Texture2D& operator=(Texture2D&& other) noexcept;

		size_t GetWidth() const {return m_Width;}
		size_t GetHeight() const {return m_Height;}
		size_t GetSize() const {return m_Data.size();}

		// Access Pixel
		T& GetPixel(size_t x, size_t y);
		const T& GetPixel(size_t x, size_t y) const;
		T& operator()(const size_t x, const size_t y) {return GetPixel(x, y);}
		const T& operator()(const size_t x, const size_t y) const {return GetPixel(x, y);}

		bool IsValidCoordinate(size_t x, size_t y) const;
		T GetPixelSafe(size_t x, size_t y, const T& defaultValue = T{}) const;
		void SetPixelSafe(size_t x, size_t y, const T& value);

		T Sample(float x, float y) const;
		T SampleWrapped(float x, float y) const;

		T* GetData(){return m_Data.data();}
		const T* GetData() const {return m_Data.data();}
		std::vector<T>& GetDataVector(){return m_Data;}
		const std::vector<T>& GetDataVector() const {return m_Data;}

		void Clear(const T& value = T{});
		void Fill(const T& value);
		void Resize(size_t w, size_t h);
		void Resize(size_t w, size_t h, const T& value);

		SDL_Texture* CreateSDLTexture(SDL_Renderer* renderer) const;
	private:
		size_t m_Width;
		size_t m_Height;
		std::vector<T> m_Data;

		size_t GetIndex(const size_t x, const size_t y) const {return y * m_Width + x;}
	};

	using Texture2D_HDR = Texture2D<glm::vec4>; // HDR
	using Texture2D_RGBA = Texture2D<uint32_t>; // SDR
	using Texture2D_S8 = Texture2D<uint8_t>;    // Stencil
	using Texture2D_RFloat = Texture2D<float>;  // 32-bit Depth/Shadowmap

	template<typename T>
	Texture2D<T>::Texture2D(const size_t w, const size_t h):
		m_Width(w), m_Height(h), m_Data(w * h)
	{

	}

	template<typename T>
	Texture2D<T>::Texture2D(const size_t w, const size_t h, const T& value):
		m_Width(w), m_Height(h), m_Data(w * h, value)
	{

	}

	template<typename T>
	Texture2D<T>::Texture2D(const Texture2D& other):
		m_Width(other.m_Width), m_Height(other.m_Height), m_Data(other.m_Data)
	{

	}

	template<typename T>
	Texture2D<T>& Texture2D<T>::operator=(const Texture2D& other)
	{
		if (this != &other)
		{
			m_Width = other.m_Width;
			m_Height = other.m_Height;
			m_Data = other.m_Data;
		}
		return *this;
	}

	template<typename T>
	Texture2D<T>::Texture2D(Texture2D&& other) noexcept:
		m_Width(other.m_Width), m_Height(other.m_Height), m_Data(std::move(other.m_Data))
	{
		other.m_Width = 0;
		other.m_Height = 0;
	}

	template<typename T>
	Texture2D<T>& Texture2D<T>::operator=(Texture2D&& other) noexcept
	{
		if (this != &other)
		{
			m_Width = other.m_Width;
			m_Height = other.m_Height;
			m_Data = std::move(other.m_Data);
			other.m_Width = 0;
			other.m_Height = 0;
		}
		return *this;
	}

	template <typename T>
	T& Texture2D<T>::GetPixel(const size_t x, const size_t y)
	{
		return m_Data[GetIndex(x, y)];
	}

	template<typename T>
	const T& Texture2D<T>::GetPixel(const size_t x, const size_t y) const
	{
		return m_Data[GetIndex(x, y)];
	}

	template<typename T>
	bool Texture2D<T>::IsValidCoordinate(const size_t x, const size_t y) const
	{
		return x < m_Width && y < m_Height;
	}

	template<typename T>
	T Texture2D<T>::GetPixelSafe(const size_t x, const size_t y, const T& defaultValue) const
	{
		if (IsValidCoordinate(x, y))
		{
			return GetPixel(x, y);
		}
		return defaultValue;
	}

	template<typename T>
	void Texture2D<T>::SetPixelSafe(const size_t x, const size_t y, const T& value)
	{
		if (IsValidCoordinate(x, y))
		{
			GetPixel(x, y) = value;
		}
	}

	template<typename T>
	T Texture2D<T>::Sample(float x, float y) const
	{
		// Clamp coordinates to texture bounds
		x = std::max(0.0f, std::min(x, static_cast<float>(m_Width - 1)));
		y = std::max(0.0f, std::min(y, static_cast<float>(m_Height - 1)));

		// Get integer coordinates
		const size_t x0 = x;
		const size_t y0 = y;
		const size_t x1 = std::min(x0 + 1, m_Width - 1);
		const size_t y1 = std::min(y0 + 1, m_Height - 1);

		// Get fractional parts
		float fx = x - static_cast<float>(x0);
		float fy = y - static_cast<float>(y0);

		// Bilinear interpolation
		T c00 = GetPixel(x0, y0);
		T c10 = GetPixel(x1, y0);
		T c01 = GetPixel(x0, y1);
		T c11 = GetPixel(x1, y1);

		// This works for arithmetic types, may need specialization for other types
		T c0 = c00 * (1.0f - fx) + c10 * fx;
		T c1 = c01 * (1.0f - fx) + c11 * fx;
		return c0 * (1.0f - fy) + c1 * fy;
	}

	template<typename T>
	T Texture2D<T>::SampleWrapped(float x, float y) const
	{
		// Wrap coordinates
		x = x - std::floor(x / static_cast<float>(m_Width)) * static_cast<float>(m_Width);
		y = y - std::floor(y / static_cast<float>(m_Height)) * static_cast<float>(m_Height);

		if (x < 0) x += static_cast<float>(m_Width);
		if (y < 0) y += static_cast<float>(m_Height);

		return Sample(x, y);
	}

	template<typename T>
	void Texture2D<T>::Clear(const T& value)
	{
		std::fill(m_Data.begin(), m_Data.end(), value);
	}

	template<typename T>
	void Texture2D<T>::Fill(const T& value)
	{
		Clear(value);
	}

	template<typename T>
	void Texture2D<T>::Resize(const size_t w, const size_t h)
	{
		m_Width = w;
		m_Height = h;
		m_Data.resize(w * h);
	}

	template<typename T>
	void Texture2D<T>::Resize(const size_t w, const size_t h, const T& value)
	{
		m_Width = w;
		m_Height = h;
		m_Data.assign(w * h, value);
	}

	template<>
	inline SDL_Texture* Texture2D<uint32_t>::CreateSDLTexture(SDL_Renderer* renderer) const
	{
		SDL_Texture* texture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STATIC,
			(int)(m_Width),
			(int)m_Height
		);

		if (texture)
		{
			SDL_UpdateTexture(texture, nullptr, m_Data.data(), (int)(m_Width * sizeof(uint32_t)));
		}

		return texture;
	}

	template<>
	inline SDL_Texture* Texture2D<float>::CreateSDLTexture(SDL_Renderer* renderer) const
	{
		std::vector<uint32_t> data(m_Data.size());

		for (size_t i = 0; i < m_Data.size(); i++)
		{
			uint8_t gray = (uint8_t)(std::clamp(m_Data[i] * 255.0f, 0.0f, 255.0f));
			data[i] = (255 << 24) | (gray << 16) | (gray << 8) | gray;
		}
		SDL_Texture* texture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STATIC,
			 (int)(m_Width),
			 (int)m_Height
		);

		if (texture)
		{
			SDL_UpdateTexture(texture, nullptr, data.data(), (int)(m_Width * sizeof(uint32_t)));
		}

		return texture;
	}

	template<>
	inline float Texture2D<float>::Sample(float x, float y) const
	{
		// Clamp coordinates to texture bounds
		x = std::max(0.0f, std::min(x, static_cast<float>(m_Width - 1)));
		y = std::max(0.0f, std::min(y, static_cast<float>(m_Height - 1)));

		// Get integer coordinates
		size_t x0 = static_cast<size_t>(x);
		size_t y0 = static_cast<size_t>(y);
		size_t x1 = std::min(x0 + 1, m_Width - 1);
		size_t y1 = std::min(y0 + 1, m_Height - 1);

		// Get fractional parts
		float fx = x - static_cast<float>(x0);
		float fy = y - static_cast<float>(y0);

		// Bilinear interpolation for float values
		float c00 = GetPixel(x0, y0);
		float c10 = GetPixel(x1, y0);
		float c01 = GetPixel(x0, y1);
		float c11 = GetPixel(x1, y1);

		// Linear interpolation
		float c0 = c00 * (1.0f - fx) + c10 * fx;
		float c1 = c01 * (1.0f - fx) + c11 * fx;
		return c0 * (1.0f - fy) + c1 * fy;
	}

	template<>
	inline SDL_Texture* Texture2D<glm::vec4>::CreateSDLTexture(SDL_Renderer* renderer) const
	{
		// Convert float RGBA to 32-bit RGBA
		std::vector<uint32_t> rgbaData(m_Data.size());
		for (size_t i = 0; i < m_Data.size(); ++i)
		{
			const glm::vec4& pixel = m_Data[i];
			uint8_t r = static_cast<uint8_t>(std::clamp(pixel.r * 255.0f, 0.0f, 255.0f));
			uint8_t g = static_cast<uint8_t>(std::clamp(pixel.g * 255.0f, 0.0f, 255.0f));
			uint8_t b = static_cast<uint8_t>(std::clamp(pixel.b * 255.0f, 0.0f, 255.0f));
			uint8_t a = static_cast<uint8_t>(std::clamp(pixel.a * 255.0f, 0.0f, 255.0f));
			rgbaData[i] = (a << 24) | (b << 16) | (g << 8) | r;
		}

		SDL_Texture* texture = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STATIC,
			static_cast<int>(m_Width),
			static_cast<int>(m_Height));

		if (texture)
		{
			SDL_UpdateTexture(texture, nullptr, rgbaData.data(), static_cast<int>(m_Width * sizeof(uint32_t)));
		}

		return texture;
	}

	template<>
	inline glm::vec4 Texture2D<glm::vec4>::Sample(float x, float y) const
	{
		// Clamp coordinates to texture bounds
		x = std::max(0.0f, std::min(x, static_cast<float>(m_Width - 1)));
		y = std::max(0.0f, std::min(y, static_cast<float>(m_Height - 1)));

		// Get integer coordinates
		size_t x0 = static_cast<size_t>(x);
		size_t y0 = static_cast<size_t>(y);
		size_t x1 = std::min(x0 + 1, m_Width - 1);
		size_t y1 = std::min(y0 + 1, m_Height - 1);

		// Get fractional parts
		float fx = x - static_cast<float>(x0);
		float fy = y - static_cast<float>(y0);

		// Bilinear interpolation for vec4 values
		glm::vec4 c00 = GetPixel(x0, y0);
		glm::vec4 c10 = GetPixel(x1, y0);
		glm::vec4 c01 = GetPixel(x0, y1);
		glm::vec4 c11 = GetPixel(x1, y1);

		// Linear interpolation (GLM handles component-wise operations)
		glm::vec4 c0 = c00 * (1.0f - fx) + c10 * fx;
		glm::vec4 c1 = c01 * (1.0f - fx) + c11 * fx;
		return c0 * (1.0f - fy) + c1 * fy;
	}

	template<>
	inline SDL_Texture* Texture2D<uint8_t>::CreateSDLTexture(SDL_Renderer* renderer) const
	{
		// Convert stencil values (0-255) to grayscale for visualization
		std::vector<uint32_t> rgbaData(m_Data.size());
		for (size_t i = 0; i < m_Data.size(); ++i)
		{
			uint8_t stencilValue = m_Data[i];
			// Display stencil as grayscale (white = 255, black = 0)
			rgbaData[i] = (255 << 24) | (stencilValue << 16) | (stencilValue << 8) | stencilValue;
		}

		SDL_Texture* texture = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STATIC,
			static_cast<int>(m_Width),
			static_cast<int>(m_Height));

		if (texture)
		{
			SDL_UpdateTexture(texture, nullptr, rgbaData.data(), static_cast<int>(m_Width * sizeof(uint32_t)));
		}

		return texture;
	}
}
