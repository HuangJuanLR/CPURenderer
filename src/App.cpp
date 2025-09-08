#include <iostream>

#include "App.h"

App::App()
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

	SDL_SetRenderLogicalPresentation(m_Renderer, m_LogicW, m_LogicH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	if (success)
		std::cout << "SDL3 Initialized" << std::endl;
}

App::~App()
{
	CleanUp();
}

int App::Run()
{
	if (!success) return 1;
	// InitWindow(1920, 1080, "Raylib");
	// SetTargetFPS(60);
	//
	// capybara = LoadModel("resources/models/icosahedron.obj");
	// electra = LoadModel("resources/models/electra.gltf");
	//
	// // std::cout << IsModelValid(capybara) << std::endl;
	//
	// if (!FileExists("resources/models/african_head.obj"))
	// {
	// 	TraceLog(LOG_WARNING, "Model file not found!");
	// }
	//
	// while (!WindowShouldClose()) {
	// 	BeginDrawing();
	// 	// DrawModel(capybara);
	// 	ClearBackground(BLACK);
	//
	// 	// graphics::Line(100, 100, 500, 200, 8, RED);
	// 	// graphics::Line(150, 100, 200, 800, 8, GREEN);
	// 	// graphics::Line(500, 50, 200, 800, 8, BLUE);
	//
	// 	Graphics::Circle(960, 540, 200, 4, BLUE);
	//
	// 	DrawText(std::format("electra.meshes: {}", electra.meshCount).c_str(), 10, 50, 30, WHITE);
	//
	// 	for (int i = 0; i < capybara.meshes[0].vertexCount; i+=3)
	// 	{
	// 		float* vertices = capybara.meshes[0].vertices;
	// 		Vector2 v0 = {vertices[i*3], vertices[i*3+1]};
	// 		Vector2 v1 = {vertices[(i + 1)*3], vertices[(i + 1)*3+1]};
	// 		Vector2 v2 = {vertices[(i + 2)*3], vertices[(i + 2)*3+1]};
	// 		v0.x += 1.0;
	// 		v0.x *= (float)1920/2;
	// 		v0.y += 1.0;
	// 		v0.y = 1.0f - v0.y + 1.0f;
	// 		v0.y *= (float)1080/2;
	//
	// 		v1.x += 1.0;
	// 		v1.x *= (float)1920/2;
	// 		v1.y += 1.0;
	// 		v1.y = 1.0f - v1.y + 1.0f;
	// 		v1.y *= (float)1080/2;
	//
	// 		v2.x += 1.0;
	// 		v2.x *= (float)1920/2;
	// 		v2.y += 1.0;
	// 		v2.y = 1.0f - v2.y + 1.0f;
	// 		v2.y *= (float)1080/2;
	//
	// 		v0.x *= (float)1080/1920;
	// 		v1.x *= (float)1080/1920;
	// 		v2.x *= (float)1080/1920;
	// 		Graphics::Line(v0, v1, 1, RED);
	// 		Graphics::Line(v1, v2, 1, GREEN);
	// 		Graphics::Line(v2, v0, 1, BLUE);
	// 	}
	//
	// 	EndDrawing();
	// }
	//
	// CloseWindow();


	DoFrame();
	CleanUp();

	return 0;
}

void App::DoFrame()
{
	uint64_t prevTime = SDL_GetPerformanceCounter();
	uint64_t frequency = SDL_GetPerformanceFrequency();

	// Game Loop
	bool running = true;
	while (running)
	{
		uint64_t nowTime = SDL_GetPerformanceCounter();
		float deltaTime = (nowTime - prevTime) / (float)frequency;

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

		// draw
		SDL_SetRenderDrawColor(m_Renderer, 20,20,20,255);
		SDL_RenderClear(m_Renderer);

		SDL_SetRenderDrawColor(m_Renderer, 200, 20, 20, 255);
		for (int i = -5; i < 5; i++)
		{
			for (int j = -5; j < 5; j++)
			{
				SDL_RenderPoint(m_Renderer, 240 + i, 135 + j);
			}
		}

		SDL_RenderPresent(m_Renderer);
	}
}

void App::CleanUp() const
{
	SDL_DestroyWindow(m_Window);
	SDL_DestroyRenderer(m_Renderer);
	SDL_Quit();
}
