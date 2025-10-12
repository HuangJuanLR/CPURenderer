#pragma once
#include "SDL3/SDL.h"
#include <glm.hpp>
#include <memory>

#include "Model.h"
#include "assimp/scene.h"

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
	private:
		App();
		~App();

		// Delete copy ctor and assign op
		App(const App&) = delete;
		App& operator=(const App&) = delete;

		void Update();
		void CleanUp() const;
	public:
		bool success = true;
	private:
		SDL_Window* m_Window;
		SDL_Renderer* m_Renderer;
		int m_Width, m_Height;
		int m_LogicW, m_LogicH;
		int m_TargetFPS;
		const bool* m_Keys;

		std::unique_ptr<Model> m_CapybaraModel;

		std::unique_ptr<Texture2D_RGBA> m_RenderTarget;
		SDL_Texture* m_DisplayTexture;

		static App* s_Instance;
	};
}