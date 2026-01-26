#include "Window.h"
#include "Context.h"
#include "plog/Log.h"

namespace CPURDR
{
	Window::Window(const WindowParams& params):
		m_Window(nullptr),
		m_Surface(nullptr),
		m_Renderer(nullptr),
		m_PresentTexture(nullptr),
		m_WindowID(0),
		m_Title(params.title),
		m_Width(params.width),
		m_Height(params.height),
		m_ShouldClose(false),
		m_VSync(params.vsync),
		m_LastPresentTime(0),
		m_RefreshRate(60.0f)
	{
		SDL_WindowFlags flags = 0;
		if (params.resizable)
		{
			flags |= SDL_WINDOW_RESIZABLE;
		}

		m_Window = SDL_CreateWindow(m_Title.c_str(), m_Width, m_Height, flags);

		if (!m_Window)
		{
			PLOG_ERROR << "Failed to create window: " << SDL_GetError() << std::endl;
			return;
		}
		m_WindowID = SDL_GetWindowID(m_Window);

		m_Renderer = SDL_CreateRenderer(m_Window, nullptr);
		if (!m_Renderer)
		{
			PLOG_ERROR << "Failed to create SDL_Renderer" << SDL_GetError() << std::endl;
		}

		SDL_DisplayID displayID = SDL_GetDisplayForWindow(m_Window);
		const SDL_DisplayMode* displayMode = SDL_GetCurrentDisplayMode(displayID);
		if (displayMode)
		{
			m_RefreshRate = displayMode->refresh_rate;
		}

		if (m_VSync)
		{
			SDL_SetRenderVSync(m_Renderer, 1);
		}

		m_Surface = SDL_GetWindowSurface(m_Window);
		CreateContext();
		m_LastPresentTime = SDL_GetPerformanceCounter();

		PLOG_INFO << "Window created: " << m_Window << "x" << m_Height << "@" << m_RefreshRate << "FPS" << std::endl;
	}

	Window::~Window()
	{
		if (m_PresentTexture)
		{
			SDL_DestroyTexture(m_PresentTexture);
			m_PresentTexture = nullptr;
		}

		if (m_Renderer)
		{
			SDL_DestroyRenderer(m_Renderer);
			m_Renderer = nullptr;
		}

		if (m_Window)
		{
			SDL_DestroyWindow(m_Window);
			m_Window = nullptr;
		}
	}

	void Window::SetTitle(const std::string& title)
	{
		m_Title = title;
		if (m_Window)
		{
			SDL_SetWindowTitle(m_Window, m_Title.c_str());
		}
	}

	void Window::CreateContext()
	{
		m_Context = std::make_shared<Context>(m_Width >> 2, m_Height >> 2);
	}

	void Window::Present()
	{
		if (!m_Context || !m_Renderer)
		{
			return;
		}

		if (m_VSync)
		{
			WaitForVSync();
		}

		const auto& framebuffer = m_Context->GetFramebuffer();
		if (!framebuffer.colorBuffer)
		{
			return;
		}
		m_PresentTexture = framebuffer.colorBuffer->CreateSDLTexture(m_Renderer);

		if (!m_PresentTexture)
		{
			PLOG_ERROR << "Failed to create present texture" << std::endl;
			return;
		}

		SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
		SDL_RenderClear(m_Renderer);
		SDL_RenderTexture(m_Renderer, m_PresentTexture, nullptr, nullptr);
		SDL_RenderPresent(m_Renderer);

		m_LastPresentTime = SDL_GetPerformanceCounter();
	}

	void Window::OnResize(int width, int height)
	{
		m_Width = width;
		m_Height = height;

		UpdateSurface();

		if (m_Context)
		{
			m_Context->ResizeFramebuffer(width >> 1, height >> 1);
		}

		if (m_ResizeCallback)
		{
			m_ResizeCallback(width, height);
		}

		PLOG_INFO << "Window resized: " << m_Width << "x" << m_Height << std::endl;
	}

	void Window::UpdateSurface()
	{
		if (m_Window)
		{
			m_Surface = SDL_GetWindowSurface(m_Window);
		}
	}

	void Window::WaitForVSync()
	{
		if (m_RefreshRate <= 0.0f)
		{
			return;
		}

		uint64_t frequency = SDL_GetPerformanceFrequency();
		double targetFrameTime = 1.0 / static_cast<double>(m_RefreshRate);

		uint64_t currentTime = SDL_GetPerformanceCounter();
		// elapsed time in seconds
		double elapsedTime = static_cast<double>(currentTime - m_LastPresentTime)
			/ static_cast<double>(frequency);

		if (elapsedTime < targetFrameTime)
		{
			// convert back to milliseconds
			double sleepTime = (targetFrameTime - elapsedTime) * 1000.0;
			SDL_Delay(static_cast<Uint32>(sleepTime));
		}
	}

}
