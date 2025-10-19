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
		bool resizable = true;
	};

	class Context;

	class Window
	{
	public:
		Window(const WindowParams& params);
		~Window();

		bool ShouldClose() const {return m_ShouldClose;}
		void SetShouldClose(const bool shouldClose) {m_ShouldClose = shouldClose;}

		int GetWidth() const {return m_Width;}
		int GetHeight() const {return m_Height;}
		const std::string& GetTitle() const {return m_Title;}
		void SetTitle(const std::string& title);

		SDL_Window* GetSDLWindow() const {return m_Window;}
		SDL_Surface* GetSurface() const {return m_Surface;}
		SDL_Renderer* GetSDLRenderer() const {return m_Renderer;}

		void CreateContext();
		std::shared_ptr<Context> GetContext() const {return m_Context;}

		void Present();

		// Callbacks for ImGui
		using ResizeCallback = std::function<void(int, int)>;
		void SetResizeCallback(ResizeCallback callback) {m_ResizeCallback = callback;}
		void OnResize(int width, int height);

	private:
		SDL_Window* m_Window;
		SDL_Surface* m_Surface;
		SDL_Renderer* m_Renderer;
		SDL_Texture* m_PresentTexture;
		uint32_t m_WindowID;

		std::string m_Title;
		int m_Width, m_Height;
		bool m_ShouldClose;
		bool m_VSync;

		std::shared_ptr<Context> m_Context;
		ResizeCallback m_ResizeCallback;

		uint64_t m_LastPresentTime;
		float m_RefreshRate;

		void UpdateSurface();
		void WaitForVSync();
	};
}
