#include <iostream>
#include <format>
#include "imgui.h"
#include "App.h"
#include "Graphics.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "Log.h"

namespace CPURDR
{
	App* App::s_Instance = nullptr;

	App::App():
		m_Keys(SDL_GetKeyboardState(nullptr))
	{
		m_Width = 1920;
		m_Height = 1080;
		m_LogicW = 1920;
		m_LogicH = 1080;

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
		m_TargetFPS = displayMode->refresh_rate;

		SDL_SetRenderLogicalPresentation(m_Renderer, m_LogicW, m_LogicH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

		if (success)
			std::cout << "SDL3 Initialized" << std::endl;

		m_CapybaraModel = std::make_unique<Model>("resources/assets/models/capybara.fbx");
	}

	App::~App()
	{
		CleanUp();
	}

	App& App::GetInstance()
	{
		if (s_Instance == nullptr)
		{
			s_Instance = new App();
		}
		return *s_Instance;
	}

	void App::Destroy()
	{
		if (s_Instance != nullptr)
		{
			delete s_Instance;
			s_Instance = nullptr;
		}
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
		uint64_t frequency = SDL_GetPerformanceFrequency();
		double targetTime = (m_TargetFPS > 0)? 1.0/static_cast<double>(m_TargetFPS) : 0.0;

		uint64_t prevFrameStart = SDL_GetPerformanceCounter();
		double fpsAcc = 0.0;
		int fpsFrames = 0;
		double fps = 0.0;
		std::string fpsStr = "FPS: 0";

		int lineX = 10;
		int lineY = 10;

		float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		// SDLRenderer3 doesn't support Multi-Viewport

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// Setup scaling
		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
		style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
		//io.ConfigDpiScaleFonts = true;        // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
		//io.ConfigDpiScaleViewports = true;    // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

		// Setup Platform/Renderer backends
		ImGui_ImplSDL3_InitForSDLRenderer(m_Window, m_Renderer);
		ImGui_ImplSDLRenderer3_Init(m_Renderer);

		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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
				ImGui_ImplSDL3_ProcessEvent(&event);

				switch (event.type)
				{
				case SDL_EVENT_QUIT:
					running = false;
					break;

				case SDL_EVENT_WINDOW_RESIZED:
					m_Width = event.window.data1;
					m_Height = event.window.data2;
					break;

				case SDL_EVENT_MOUSE_MOTION:
					break;
				}
			}

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

			if (m_Keys[SDL_SCANCODE_SPACE])
			{
				PLOG_VERBOSE << "This is a VERBOSE message";
				PLOG_DEBUG << "This is a DEBUG message";
				PLOG_INFO << "This is an INFO message";
				PLOG_WARNING << "This is a WARNING message";
				PLOG_ERROR << "This is an ERROR message";
				PLOG_FATAL << "This is a FATAL message";
			}

			// draw
			SDL_SetRenderDrawColor(m_Renderer, 20,20,20,255);
			SDL_RenderClear(m_Renderer);

			// SDL_SetRenderDrawColor(m_Renderer, 50, 50, 50, 255);

			// Graphics::Grid(m_Renderer, m_LogicW, m_LogicH, 10);

			// SDL_SetRenderDrawColor(m_Renderer, 200, 50, 50, 255);
			// Graphics::Line(m_Renderer, 240, 135, lineX, lineY, 1);

			// SDL_SetRenderDrawColor(m_Renderer, 50, 200, 50, 255);
			// glm::vec3 p0 = glm::vec3(240.f, 135.f, 0.0f);
			// glm::vec3 p1 = glm::vec3((float)lineX, (float)lineY, 0.0f);
			// glm::vec3 p2 = glm::vec3(240.f, 50.f, 0.0f);

			// Graphics::Triangle(m_Renderer, p0, p1, p2);

			// model

			SDL_SetRenderDrawColor(m_Renderer, 255,255,255,255);
			const auto& mesh = m_CapybaraModel->GetMeshes()[0];
			std::string modelInfo = std::format("Model: {}", mesh.vertices.size());
			SDL_RenderDebugText(m_Renderer, 5, 15, modelInfo.c_str());

			m_CapybaraModel->DrawTriangle(m_Renderer);

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

			// Start the Dear ImGui frame
			ImGui_ImplSDLRenderer3_NewFrame();
			ImGui_ImplSDL3_NewFrame();
			ImGui::NewFrame();

			if (show_demo_window)
				ImGui::ShowDemoWindow(&show_demo_window);

			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
			{
				static float f = 0.0f;
				static int counter = 0;

				ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

				ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
				ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
				ImGui::Checkbox("Another Window", &show_another_window);

				ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

				if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
					counter++;
				ImGui::SameLine();
				ImGui::Text("counter = %d", counter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
				ImGui::End();
			}

			// 3. Show another simple window.
			if (show_another_window)
			{
				ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
				ImGui::Text("Hello from another window!");
				if (ImGui::Button("Close Me"))
					show_another_window = false;
				ImGui::End();
			}

			// Rendering
			ImGui::Render();
			SDL_SetRenderScale(m_Renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
			SDL_SetRenderDrawColorFloat(m_Renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			SDL_RenderClear(m_Renderer);
			ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_Renderer);

			SDL_RenderPresent(m_Renderer);
		}
	}

	void App::CleanUp() const
	{
		// Cleanup ImGui
		ImGui_ImplSDLRenderer3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();

		SDL_DestroyWindow(m_Window);
		SDL_DestroyRenderer(m_Renderer);
		SDL_Quit();
	}
}
