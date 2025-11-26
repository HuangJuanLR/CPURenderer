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
#include "entt.hpp"
#include "Scene.h"
#include "ecs/components/Hierarchy.h"
#include "ecs/components/NameTag.h"
#include "ecs/systems/RenderingSystem.h"
#include "ecs/systems/TransformSystem.h"

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

		// Give ECS a try
		m_Scene = SceneManager::GetInstance().CreateScene("Main Scene");
		m_RenderingSystem = std::make_unique<RenderingSystem>();
		m_TransformSystem = std::make_unique<TransformSystem>();

		// entt::entity teapotEntity = m_Scene->CreateMeshEntity("Teapot", "resources/assets/models/utah_teapot.obj");

		entt::entity parentEntity = m_Scene->CreateEntity("Parent");
		auto& parentTransform = m_Scene->GetRegistry().get<Transform>(parentEntity);
		parentTransform.position = glm::vec3(0.0f, -0.5f, 0.0f);
		parentTransform.scale = glm::vec3(2.0f, 2.0f, 2.0f);

		entt::entity childEntity = m_Scene->CreateMeshEntity("Child", "resources/assets/models/utah_teapot.obj");
		auto& childTransform = m_Scene->GetRegistry().get<Transform>(childEntity);
		childTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);
		childTransform.scale = glm::vec3(0.5f, 0.5f, 0.5f);

		m_Scene->SetParent(childEntity, parentEntity);

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

			m_TransformSystem->Update(m_Scene->GetRegistry());

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

			m_RenderingSystem->Render(m_Scene->GetRegistry(), m_RenderContext.get(), *m_Camera);

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
			// Setup Dockspace
			// ========================================
			ImGuiViewport* mainViewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(mainViewport->WorkPos);
			ImGui::SetNextWindowSize(mainViewport->WorkSize);
			ImGui::SetNextWindowViewport(mainViewport->ID);

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
			windowFlags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			ImGui::Begin("DockSpaceWindow", nullptr, windowFlags);
			ImGui::PopStyleVar(3);

			ImGuiID dockspaceID = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

			SetupDockingLayout();

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Exit"))
					{
						m_RenderWindow->SetShouldClose(true);
					}
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("View"))
				{
					ImGui::MenuItem("Demo Window", nullptr, &show_demo_window);
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			ImGui::End();

			// ========================================
			// Render context to ImGui Window
			// ========================================
			{
				ImGui::Begin("Scene"); // Should match ImGui window name

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

				auto view = m_Scene->View<MeshFilter>();
				size_t totalVertices = 0;
				for (auto entity: view)
				{
					const MeshFilter& meshFilter = view.get<MeshFilter>(entity);
					for (const auto& mesh: meshFilter.meshes)
					{
						totalVertices += mesh.vertices.size();
					}
				}

				ImGui::Separator();
				ImGui::Text("Render Stats: ");
				ImGui::Text("Scene: %zu entities", m_Scene->GetEntityCount());
				ImGui::Text("Triangles: %zu", totalVertices);
				ImGui::Text(" FPS: %.1f", fps);
				ImGui::Text(" Framebuffer: %dx%d",
					m_RenderContext->GetFramebufferWidth(),
					m_RenderContext->GetFramebufferHeight());
				ImGui::Text(" In Render Pass: %s", m_RenderContext->IsInRenderPass()? "Yes" : "No");

				const Viewport& vp = m_RenderContext->GetViewport();
				ImGui::Text(" Viewport: (%d, %d) %dx%d", vp.x, vp.y, vp.width, vp.height);

				ImGui::End();
			}

			RenderSceneHierarchy();

			if (show_demo_window)
				ImGui::ShowDemoWindow(&show_demo_window);

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

	void App::RenderSceneHierarchy()
	{
		ImGui::Begin("Scene Hierarchy");

		if (!m_Scene)
		{
			ImGui::Text("No active scene");
			ImGui::End();
			return;
		}

		ImGui::Text("Scene: %s", m_Scene->GetName().c_str());
		ImGui::Text("Total Entities: %zu", m_Scene->GetEntityCount());
		ImGui::Separator();

		std::function<void(entt::entity)> renderEntityNode;
		renderEntityNode = [&](entt::entity entity)
		{
			if (!m_Scene->IsValidEntity(entity)) return;

			auto* nameTag = m_Scene->GetRegistry().try_get<NameTag>(entity);
			std::string entityName = nameTag? nameTag->name : "Unnamed Entity";

			uint32_t entityID = static_cast<uint32_t>(entity);
			std::string label = std::format("{} (ID: {})", entityName, entityID);

			auto* hierarchy = m_Scene->GetRegistry().try_get<Hierarchy>(entity);
			bool hasChildren = hierarchy && hierarchy->HasChildren();

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
									ImGuiTreeNodeFlags_OpenOnDoubleClick |
										ImGuiTreeNodeFlags_SpanAvailWidth;

			if (!hasChildren)
			{
				flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			}

			bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)entityID, flags, "%s", label.c_str());

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("Entity ID: %u", entityID);

				if (auto* transform = m_Scene->GetRegistry().try_get<Transform>(entity))
				{
					ImGui::Text("Transform: (%.2f, %.2f, %.2f)",
						transform->position.x, transform->position.y, transform->position.z);
				}

				if (auto* meshFilter = m_Scene->GetRegistry().try_get<MeshFilter>(entity))
				{
					ImGui::Text("MeshFilter: %zu meshes", meshFilter->meshes.size());
				}

				if (m_Scene->GetRegistry().try_get<MeshRenderer>(entity))
				{
					ImGui::Text("MeshRenderer");
				}

				ImGui::EndTooltip();
			}

			if (hasChildren && nodeOpen)
			{
				for (entt::entity child: hierarchy->children)
				{
					renderEntityNode(child);
				}
				ImGui::TreePop();
			}
		};

		auto rootEntities = m_Scene->GetRootEntities();

		if (rootEntities.empty())
		{
			ImGui::TextDisabled("No entities in scene");
		}
		else
		{
			ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Root Entities: %zu", rootEntities.size());
			ImGui::Separator();

			for (entt::entity root: rootEntities)
			{
				renderEntityNode(root);
			}
		}

		ImGui::End();
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

	void App::SetupDockingLayout()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");

		if (!m_DockingLayoutInitialized)
		{
			m_DockingLayoutInitialized = true;

			ImGui::DockBuilderRemoveNode(dockspaceId);
			ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->Size);

			ImGuiID dockLeft, dockRight;
			ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.25f, &dockLeft, &dockRight);

			ImGui::DockBuilderDockWindow("Scene Hierarchy", dockLeft);
			ImGui::DockBuilderDockWindow("Scene", dockRight);

			ImGui::DockBuilderFinish(dockspaceId);
		}
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
