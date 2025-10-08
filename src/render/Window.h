#pragma once
#include <functional>
#include <memory>
#include <string>
#include "SDL3/SDL.h"

namespace CPURDR
{
	struct WindowParams
	{
		std::string title = "Render Window";
		int width = 1280;
		int height = 720;
		bool vsync = false;
	};

	class Context;

	class Window
	{
	public:
		Window(const WindowParams& params);
		~Window();

		bool ShouldClose() const {return m_ShouldClose;}
		void SetShouldClose(const bool shouldClose) {m_ShouldClose = shouldClose;}

		SDL_Window* GetSDLWindow() const {return m_Window;}
		SDL_Surface* GetSurface() const {return m_Surface;}

		// Callbacks for ImGui
		using ResizeCallback = std::function<void(int, int)>;
		void SetResizeCallback(ResizeCallback callback) {m_ResizeCallback = callback;}

	private:
		SDL_Window* m_Window;
		SDL_Surface* m_Surface;
		uint32_t m_WindowID;

		std::string m_Title;
		int m_Width, m_Height;
		bool m_ShouldClose;
		bool m_VSync;

		std::shared_ptr<Context> m_Context;
		ResizeCallback m_ResizeCallback;

		uint64_t m_LastPresentTime;
		double m_RefreshRate;

		void UpdateSurface();
		void WaitForVSync();
	};
}
