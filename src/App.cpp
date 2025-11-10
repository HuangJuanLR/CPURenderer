#include <iostream>
#include <format>
#include "imgui.h"
#include "App.h"
#include "Graphics.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "imgui_internal.h"
#include "Log.h"
#include "render/Context.h"

namespace CPURDR
{
	App* App::s_Instance = nullptr;

	App::App():
		m_Keys(SDL_GetKeyboardState(nullptr)),
		m_ImGuiRenderer(nullptr),
		m_DisplayTexture(nullptr)
	{
		m_Width = 2560;
		m_Height = 1440;
		m_LogicW = 2560;
		m_LogicH = 1440;

		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
			success = false;
			return;
		}

		WindowParams windowParams;
		windowParams.title = "CPURenderer";
		windowParams.width = 2560;
		windowParams.height = 1440;
		windowParams.vsync = false;
		windowParams.resizable = true;

		m_RenderWindow = std::make_unique<Window>(windowParams);
		if (!m_RenderWindow->GetSDLWindow())
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
			success = false;
			return;
		}
		m_RenderContext = m_RenderWindow->GetContext();
		m_ImGuiRenderer = m_RenderWindow->GetSDLRenderer();

		m_RenderWindow->SetResizeCallback([this](int width, int height)
		{
			m_LogicW = width;
			m_LogicH = height;
		});

		if (success)
			PLOG_INFO << "SDL3 Initialized" << std::endl;

		m_CapybaraModel = std::make_unique<Model>("resources/assets/models/utah_teapot.obj");

		m_Camera = std::make_unique<Camera>(
			glm::vec3(1.5f, 1.5f, 1.5f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			0.0f,
			0.0f
			);
		m_Camera->LookAt(glm::vec3(0,0,0));
		m_Camera->SetFOV(60.0f);
		m_Camera->SetClipPlanes(0.2f, 4.0f);

		InitImGui();
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

		uint64_t prevFrameStart = SDL_GetPerformanceCounter();
		double fpsAcc = 0.0;
		int fpsFrames = 0;
		double fps = 0.0;
		std::string fpsStr = "FPS: 0";

		int lineX = 10;
		int lineY = 10;

		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		// Game Loop
		while (!m_RenderWindow->ShouldClose())
		{
			uint64_t frameStart = SDL_GetPerformanceCounter();
			double deltaTime = static_cast<double>(frameStart - prevFrameStart) / static_cast<double>(frequency);
			prevFrameStart = frameStart;

			SDL_Event event{0};
			while (SDL_PollEvent(&event))
			{
				if (!m_MouseCaptured)
				{
					ImGui_ImplSDL3_ProcessEvent(&event);
				}

				switch (event.type)
				{
					case SDL_EVENT_QUIT:
						m_RenderWindow->SetShouldClose(true);
						break;

					case SDL_EVENT_WINDOW_RESIZED:
						m_RenderWindow->OnResize(event.window.data1, event.window.data2);
						break;

					case SDL_EVENT_MOUSE_MOTION:
						if (m_MouseCaptured)
						{
							float xOffset = static_cast<float>(event.motion.xrel);
							float yOffset = static_cast<float>(event.motion.yrel);

							m_Camera->ProcessMouseMovement(xOffset, -yOffset);
						}
						break;

					case SDL_EVENT_MOUSE_BUTTON_DOWN:
						if (event.button.button == SDL_BUTTON_RIGHT)
						{
							m_MouseCaptured = true;
							SDL_SetWindowRelativeMouseMode(m_RenderWindow->GetSDLWindow(), true);
						}
						break;

					case SDL_EVENT_MOUSE_BUTTON_UP:
						if (event.button.button == SDL_BUTTON_RIGHT)
						{
							m_MouseCaptured = false;
							SDL_SetWindowRelativeMouseMode(m_RenderWindow->GetSDLWindow(), false);
						}
						break;
				}
			}

			bool moveForward = m_Keys[SDL_SCANCODE_W];
			bool moveBackward = m_Keys[SDL_SCANCODE_S];
			bool moveLeft = m_Keys[SDL_SCANCODE_A];
			bool moveRight = m_Keys[SDL_SCANCODE_D];
			bool sprint = m_Keys[SDL_SCANCODE_LSHIFT];

			ImGuiIO& io = ImGui::GetIO();
			// if (!io.WantCaptureMouse && !io.WantCaptureKeyboard)
			// {
				m_Camera->ProcessKeyboard(static_cast<float>(deltaTime),
										  moveForward, moveBackward,
										  moveLeft, moveRight,
										  sprint);
			// }

			ClearValue clearValue;
			clearValue.color = 0x141414FF;
			// ============================
			// Standard-Z
			// ============================
			clearValue.depth = 1.0f;
			// ============================
			// Reversed-Z
			// ============================
			// clearValue.depth = 0.0f;
			m_RenderContext->BeginRenderPass(clearValue);

			Viewport viewport;
			viewport.x = 0;
			viewport.y = 0;
			viewport.width = m_RenderContext->GetFramebufferWidth();
			viewport.height = m_RenderContext->GetFramebufferHeight();
			m_RenderContext->SetViewport(viewport);

			m_CapybaraModel->Draw(m_RenderContext.get(), *m_Camera);

			m_RenderContext->EndRenderPass();

			const Texture2D_RGBA* contextColorBuffer = m_RenderContext->GetColorBuffer();
			if (m_DisplayTexture)
			{
				SDL_DestroyTexture(m_DisplayTexture);
				m_DisplayTexture = nullptr;
			}

			if (contextColorBuffer)
			{
				m_DisplayTexture = contextColorBuffer->CreateSDLTexture(m_ImGuiRenderer);
			}

			SDL_SetRenderDrawColor(m_ImGuiRenderer, 20, 20, 20, 255);
			SDL_RenderClear(m_ImGuiRenderer);

			// FPS
			fpsAcc += deltaTime;
			fpsFrames++;
			if (fpsAcc >= 0.05)
			{
				fps = static_cast<double>(fpsFrames) / fpsAcc;
				fpsAcc = 0.0;
				fpsFrames = 0;
			}

			// Start the Dear ImGui frame
			ImGui_ImplSDLRenderer3_NewFrame();
			ImGui_ImplSDL3_NewFrame();
			ImGui::NewFrame();

			// ========================================
			// Render context to ImGui Window
			// ========================================
			{
				ImGui::Begin("CPURDR Output");

				if (m_DisplayTexture)
				{
					ImVec2 size = ImGui::GetContentRegionAvail();

					float aspect = (float)m_LogicW / (float)m_LogicH;
					float displayWidth = size.x;
					float displayHeight = size.x / aspect;
					if (displayHeight > size.y)
					{
						displayHeight = size.y;
						displayWidth = displayHeight * aspect;
					}
					ImGui::Image(m_DisplayTexture, ImVec2(displayWidth, displayHeight));
				}

				ImGui::Separator();
				ImGui::Text("Render Stats: ");
				ImGui::Text(" Model: %zu vertices", m_CapybaraModel->GetMeshes()[0].vertices.size());
				ImGui::Text(" FPS: %.1f", fps);
				ImGui::Text(" Framebuffer: %dx%d",
					m_RenderContext->GetFramebufferWidth(),
					m_RenderContext->GetFramebufferHeight());
				ImGui::Text(" In Render Pass: %s", m_RenderContext->IsInRenderPass()? "Yes" : "No");

				const Viewport& vp = m_RenderContext->GetViewport();
				ImGui::Text(" Viewport: (%d, %d) %dx%d", vp.x, vp.y, vp.width, vp.height);

				ImGui::End();
			}

			if (show_demo_window)
				ImGui::ShowDemoWindow(&show_demo_window);

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

				ImGuiIO& io = ImGui::GetIO();
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
			// ImGuiIO& io = ImGui::GetIO();
			SDL_SetRenderScale(m_ImGuiRenderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
			SDL_SetRenderDrawColorFloat(m_ImGuiRenderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			SDL_RenderClear(m_ImGuiRenderer);
			ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_ImGuiRenderer);

			SDL_RenderPresent(m_ImGuiRenderer);
		}
	}

	void App::InitImGui()
	{
		float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(main_scale);
		style.FontScaleDpi = main_scale;

		ImGui_ImplSDL3_InitForSDLRenderer(m_RenderWindow->GetSDLWindow(), m_ImGuiRenderer);
		ImGui_ImplSDLRenderer3_Init(m_ImGuiRenderer);
	}

	void App::ShutdownImGui()
	{
		ImGui_ImplSDLRenderer3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
	}

	void App::CleanUp()
	{
		if (m_DisplayTexture)
		{
			SDL_DestroyTexture(m_DisplayTexture);
			m_DisplayTexture = nullptr;
		}

		ShutdownImGui();

		SDL_Quit();
	}
}
