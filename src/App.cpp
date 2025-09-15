#include <iostream>
#include <format>

#include "App.h"
#include "Graphics.h"

App::App():
	m_Keys(SDL_GetKeyboardState(nullptr)),
	m_CapybaraModel("resources/assets/models/capybara.fbx")
{
	m_Width = 1920;
	m_Height = 1080;
	m_LogicW = 480;
	m_LogicH = 270;

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
		success = false;
	}

	m_Window = SDL_CreateWindow("CPURenderer", m_Width, m_Height, SDL_WINDOW_RESIZABLE);
	if (!m_Window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", m_Window);
		CleanUp();
		success = false;
	}

	m_Renderer = SDL_CreateRenderer(m_Window, nullptr);
	if (!m_Renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", m_Window);
		CleanUp();
		success = false;
	}
	// SDL_SetRenderVSync(m_Renderer, SDL_RENDERER_VSYNC_ADAPTIVE);

	SDL_DisplayID displayID = SDL_GetDisplayForWindow(m_Window);
	const SDL_DisplayMode* displayMode = SDL_GetCurrentDisplayMode(displayID);
	m_TargetFPS = displayMode->refresh_rate / 4;

	SDL_SetRenderLogicalPresentation(m_Renderer, m_LogicW, m_LogicH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	if (success)
		std::cout << "SDL3 Initialized" << std::endl;
}

App::~App()
{
	CleanUp();
}

int App::Start()
{
	if (!success) return 1;
	Update();
	CleanUp();
	return 0;
}

void App::Update()
{
	uint64_t prevTime = SDL_GetPerformanceCounter();
	uint64_t frequency = SDL_GetPerformanceFrequency();
	double targetTime = (m_TargetFPS > 0)? 1.0/static_cast<double>(m_TargetFPS) : 0.0;

	uint64_t prevFrameStart = SDL_GetPerformanceCounter();
	double fpsAcc = 0.0;
	int fpsFrames = 0;
	double fps = 0.0;
	std::string fpsStr = "FPS: 0";

	int lineX = 10;
	int lineY = 10;

	// Game Loop
	bool running = true;
	while (running)
	{
		uint64_t frameStart = SDL_GetPerformanceCounter();
		double deltaTime = static_cast<double>(frameStart - prevFrameStart) / static_cast<double>(frequency);
		prevFrameStart = frameStart;

		SDL_Event event{0};
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_EVENT_QUIT:
					running = false;
					break;

				case SDL_EVENT_WINDOW_RESIZED:
					m_Width = event.window.data1;
					m_Height = event.window.data2;
					break;
			}
		}

		if (m_Keys[SDL_SCANCODE_W])
		{
			lineY-=10;
		}
		if (m_Keys[SDL_SCANCODE_A])
		{
			lineX-=10;
		}
		if (m_Keys[SDL_SCANCODE_S])
		{
			lineY+=10;
		}
		if (m_Keys[SDL_SCANCODE_D])
		{
			lineX+=10;
		}

		// draw
		SDL_SetRenderDrawColor(m_Renderer, 20,20,20,255);
		SDL_RenderClear(m_Renderer);

		SDL_SetRenderDrawColor(m_Renderer, 50, 50, 50, 255);

		Graphics::Grid(m_Renderer, m_LogicW, m_LogicH, 10);

		SDL_SetRenderDrawColor(m_Renderer, 200, 50, 50, 255);
		Graphics::Line(m_Renderer, 240, 135, lineX, lineY, 5);

		// model
		SDL_SetRenderDrawColor(m_Renderer, 255,255,255,255);
		const auto& mesh = m_CapybaraModel.GetMeshes()[0];
		std::string modelInfo = std::format("Model: {}", mesh.vertices.size());
		SDL_RenderDebugText(m_Renderer, 5, 15, modelInfo.c_str());

		// FPS
		fpsAcc += deltaTime;
		fpsFrames++;
		if (fpsAcc >= 0.05)
		{
			fps = static_cast<double>(fpsFrames) / fpsAcc;
			fpsAcc = 0.0;
			fpsFrames = 0;
			fpsStr = std::format("FPS: {:.1f}", fps);
		}
		SDL_SetRenderDrawColor(m_Renderer, 255,255,255,255);
		SDL_RenderDebugText(m_Renderer, 5, 5, fpsStr.c_str());

		SDL_RenderPresent(m_Renderer);

		if(targetTime > 0.0)
		{
			uint64_t frameEnd = SDL_GetPerformanceCounter();
			double frameTime = static_cast<double>(frameEnd - frameStart)/static_cast<double>(frequency);
			if (frameTime < targetTime)
			{
				double sleep = (targetTime - frameTime) * 1000.0;
				SDL_Delay(static_cast<Uint32>(sleep));
			}
		}
	}
}

void App::CleanUp() const
{
	SDL_DestroyWindow(m_Window);
	SDL_DestroyRenderer(m_Renderer);
	SDL_Quit();
}
