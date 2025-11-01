#pragma once
#include "SDL3/SDL.h"
#include <glm.hpp>
#include <memory>

#include "Model.h"
#include "assimp/scene.h"
#include "render/Window.h"

namespace CPURDR
{
	class App
	{
	public:
		static App& GetInstance();
		static void Destroy();
		int Start();

		int GetWidth() const {return m_Width;}
		int GetHeight() const {return m_Height;}
		int GetLogicWidth() const {return m_LogicW;}
		int GetLogicHeight() const {return m_LogicH;}

		Window* GetWindow() const {return m_RenderWindow.get();}
		Context* GetContext() const {return m_RenderContext.get();}
	private:
		App();
		~App();

		// Delete copy ctor and assign op
		App(const App&) = delete;
		App& operator=(const App&) = delete;

		void Update();
		void CleanUp();
		void InitImGui();
		void ShutdownImGui();

	public:
		bool success = true;
	private:
		std::unique_ptr<Window> m_RenderWindow;
		std::shared_ptr<Context> m_RenderContext;

		int m_Width, m_Height;
		int m_LogicW, m_LogicH;
		const bool* m_Keys;

		std::unique_ptr<Model> m_CapybaraModel;
		std::unique_ptr<Camera> m_Camera;

		SDL_Renderer* m_ImGuiRenderer;
		SDL_Texture* m_DisplayTexture;

		static App* s_Instance;
	};
}
