#include <iostream>
#include <format>
#include "imgui.h"
#include "gtc/type_ptr.hpp"
#include "App.h"

#include "ComponentReflection.h"
#include "Gizmos.h"
#include "Graphics.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "imgui_internal.h"
#include "Log.h"
#include "MaterialPropertyInspector.h"
#include "MetaInspector.h"
#include "render/Context.h"
#include "Scene.h"
#include "Primitives.h"
#include "ecs/components/Hierarchy.h"
#include "ecs/components/NameTag.h"
#include "ecs/systems/TransformSystem.h"
#include "render/MaterialManager.h"
#include "render/ShaderManager.h"

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

		m_Scene = SceneManager::GetInstance().CreateScene("Main Scene");

		ShaderManager::GetInstance().Initialize();
		MaterialManager::GetInstance().Initialize();
		m_RenderPipeline = std::make_unique<RenderPipeline>();

		m_TransformSystem = std::make_unique<TransformSystem>();

		m_Camera = std::make_unique<Camera>(
			glm::vec3(0.0f, 0.0f, 5.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			0.0f,
			00.0f
			);
		m_Camera->LookAt(glm::vec3(0,0,0));
		// m_Camera->SetRotation(0.0f, 0.0f, 0.0f);
		m_Camera->SetFOV(60.0f);
		m_Camera->SetClipPlanes(0.1f, 100.0f);

		InitImGui();
		RegisterComponentReflection();
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

		bool show_demo_window = false;
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
			m_Camera->ProcessKeyboard(static_cast<float>(deltaTime),
									  moveForward, moveBackward,
									  moveLeft, moveRight,
									  sprint);

			m_TransformSystem->Update(m_Scene->GetRegistry());

			HandleEntityDeletion();

			// ==================================
			// Begin Rendering
			// ==================================
			ClearValue clearValue;
			clearValue.color = 0x141414FF;
			// Standard-Z
			clearValue.depth = 1.0f;
			// Reversed-Z
			// clearValue.depth = 0.0f;
			m_RenderContext->BeginRenderPass(clearValue);

			Viewport viewport;
			viewport.x = 0;
			viewport.y = 0;
			viewport.width = m_RenderContext->GetFramebufferWidth();
			viewport.height = m_RenderContext->GetFramebufferHeight();
			m_RenderContext->SetViewport(viewport);

			m_RenderPipeline->Render(m_Scene->GetRegistry(), m_RenderContext.get(), *m_Camera);

			Gizmos::DrawAxis(m_RenderContext.get(), *m_Camera, 2.0f, 0.03f);

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

			// =========================
			// GUI Content
			// =========================
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

				if (ImGui::BeginMenu("GameObject"))
				{
					if (ImGui::MenuItem("Create Empty"))
					{
						glm::vec3 spawnPosition = m_Camera->GetPosition() + m_Camera->GetFront() * 10.0f;
						entt::entity entity = m_Scene->CreateEntity("Empty Entity");
						auto& transform = m_Scene->GetRegistry().get<Transform>(entity);
						transform.position = spawnPosition;
						transform.scale = glm::vec3(1.0f);
						transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
					}

					ImGui::Separator();

					if (ImGui::BeginMenu("3D Object"))
					{
						glm::vec3 spawnPosition = m_Camera->GetPosition() + m_Camera->GetFront() * 10.0f;

						auto createPrimitiveEntity = [&](const std::string& name, const Mesh& mesh, float scale = 1.0f) {
							entt::entity entity = m_Scene->CreateMeshEntity(name, MeshFilter({mesh}));
							auto& transform = m_Scene->GetRegistry().get<Transform>(entity);
							transform.position = spawnPosition;
							transform.scale = glm::vec3(scale);
							transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
						};

						if (ImGui::MenuItem("Cube"))      createPrimitiveEntity("Cube", Primitives::Cube());
						if (ImGui::MenuItem("Sphere"))    createPrimitiveEntity("Sphere", Primitives::Sphere());
						if (ImGui::MenuItem("Plane"))     createPrimitiveEntity("Plane", Primitives::Plane(), 10.0f);
						if (ImGui::MenuItem("Quad"))      createPrimitiveEntity("Quad", Primitives::Quad());
						if (ImGui::MenuItem("Cylinder"))  createPrimitiveEntity("Cylinder", Primitives::Cylinder());
						if (ImGui::MenuItem("Capsule"))   createPrimitiveEntity("Capsule", Primitives::Capsule());

						ImGui::EndMenu();
					}

					ImGui::Separator();

					if (ImGui::BeginMenu("Light"))
					{
						glm::vec3 spawnPosition = glm::vec3(0);

						if (ImGui::MenuItem("Directional Light"))
						{
							entt::entity entity = m_Scene->CreateDirectionalLightEntity("Directional Light");
							auto& transform = m_Scene->GetRegistry().get<Transform>(entity);
							transform.position = spawnPosition;
						}

						ImGui::BeginDisabled();
						ImGui::MenuItem("Point Light");
						ImGui::MenuItem("Spot Light");
						ImGui::MenuItem("Area Light");
						ImGui::EndDisabled();

						ImGui::EndMenu();
					}

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

			RenderInspector();

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

    static entt::entity draggedEntity = entt::null;
    static std::vector<entt::entity> draggedEntities;
    static bool isDragging = false;

    struct InsertionPoint
    {
        bool active = false;
        ImVec2 lineStart;
        ImVec2 lineEnd;
        size_t insertIndex;
        entt::entity insertParent = entt::null;
    };
    static InsertionPoint insertion;

    // Store item bounds for post-processing
    struct ItemBounds
    {
        ImVec2 min;
        ImVec2 max;
        entt::entity entity;
        entt::entity parent;
        size_t indexInParent;
    };
    std::vector<ItemBounds> allItemBounds;

    // Reset insertion each frame
    insertion.active = false;

    std::function<void(entt::entity, size_t, entt::entity)> renderEntityNode;
    renderEntityNode = [&](entt::entity entity, size_t indexInParent, entt::entity parent)
    {
        if (!m_Scene->IsValidEntity(entity)) return;

        auto* nameTag = m_Scene->GetRegistry().try_get<NameTag>(entity);
        std::string entityName = nameTag ? nameTag->name : "Unnamed Entity";

        auto entityID = static_cast<uint32_t>(entity);
        std::string label = std::format("{} (ID: {})", entityName, entityID);

        auto* hierarchy = m_Scene->GetRegistry().try_get<Hierarchy>(entity);
        bool hasChildren = hierarchy && hierarchy->HasChildren();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                ImGuiTreeNodeFlags_SpanAvailWidth;

        if (IsEntitySelected(entity))
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        if (!hasChildren)
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)entityID, flags, "%s", label.c_str());

        // Store item bounds for later processing
        ItemBounds bounds;
        bounds.min = ImGui::GetItemRectMin();
        bounds.max = ImGui::GetItemRectMax();
        bounds.entity = entity;
        bounds.parent = parent;
        bounds.indexInParent = indexInParent;
        allItemBounds.push_back(bounds);

        // Handle clicking
        if (ImGui::IsItemClicked() && !isDragging)
        {
            ImGuiIO& io = ImGui::GetIO();
            bool multiSelect = io.KeyCtrl || io.KeyShift;

            if (multiSelect)
            {
                if (IsEntitySelected(entity))
                {
                    DeselectEntity(entity);
                }
                else
                {
                    SelectEntity(entity, true);
                }
            }
            else
            {
            	if (!IsEntitySelected(entity))
            	{
            		SelectEntity(entity, false);
            	}
            }
        }

    	if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !isDragging)
    	{
    		ImGuiIO& io = ImGui::GetIO();
    		bool multiSelect = io.KeyCtrl || io.KeyShift;

    		if (!multiSelect && IsEntitySelected(entity) && m_SelectedEntities.size() > 1)
    		{
    			SelectEntity(entity, false);
    		}
    	}

        // Drag source
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            if (IsEntitySelected(entity))
            {
                draggedEntities = m_SelectedEntities;
                ImGui::SetDragDropPayload("DND_ENTITIES", m_SelectedEntities.data(),
                                        m_SelectedEntities.size() * sizeof(entt::entity));
                ImGui::Text("Moving %zu entities", m_SelectedEntities.size());
            }
            else
            {
                draggedEntities = {entity};
                ImGui::SetDragDropPayload("DND_ENTITY", &entity, sizeof(entt::entity));
                ImGui::Text("Moving: %s", entityName.c_str());
            }
            draggedEntity = entity;
            isDragging = true;
            ImGui::EndDragDropSource();
        }

        // Drop target for parenting (center of item)
        if (ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ENTITY");
            if (!payload)
            {
                payload = ImGui::AcceptDragDropPayload("DND_ENTITIES");
            }

            if (payload)
            {
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->AddRect(bounds.min, bounds.max, IM_COL32(0, 255, 0, 200), 0.0f, 0, 2.0f);

                if (payload->IsDelivery())
                {
                    size_t count = payload->DataSize / sizeof(entt::entity);
                    entt::entity* entities = (entt::entity*)payload->Data;

                    for (size_t i = 0; i < count; i++)
                    {
                        if (entities[i] != entity)
                        {
                            m_Scene->SetParent(entities[i], entity);
                        }
                    }
                    PLOG_INFO << "Parented " << count << " entities to " << entityName;
                }
            }
            ImGui::EndDragDropTarget();
        }

        // Tooltip
        if (ImGui::IsItemHovered() && !isDragging)
        {
            ImGui::BeginTooltip();
            ImGui::Text("Entity ID: %u", entityID);

            if (auto* transform = m_Scene->GetRegistry().try_get<Transform>(entity))
            {
                ImGui::Text("Transform: (%.2f, %.2f, %.2f)",
                    transform->position.x, transform->position.y, transform->position.z);
            }
            ImGui::EndTooltip();
        }

        if (hasChildren && nodeOpen)
        {
            std::vector<entt::entity> sortedChildren = hierarchy->children;
            std::sort(sortedChildren.begin(), sortedChildren.end(),
                [this](entt::entity a, entt::entity b)
                {
                    return m_Scene->GetEntityOrder(a) < m_Scene->GetEntityOrder(b);
                });

            for (size_t i = 0; i < sortedChildren.size(); i++)
            {
                renderEntityNode(sortedChildren[i], i, entity);
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

        for (size_t i = 0; i < rootEntities.size(); i++)
        {
            renderEntityNode(rootEntities[i], i, entt::null);
        }
    }

    // Post-process: Check mouse position against all item bounds for insertion
    if (isDragging && !allItemBounds.empty())
    {
        ImVec2 mousePos = ImGui::GetMousePos();

        for (const auto& bounds : allItemBounds)
        {
            // Skip the entity being dragged
            bool isDraggedEntity = std::find(draggedEntities.begin(), draggedEntities.end(),
                                            bounds.entity) != draggedEntities.end();
            if (isDraggedEntity) continue;

            float itemHeight = bounds.max.y - bounds.min.y;
            float insertZoneHeight = itemHeight * 0.3f;

            // Check if mouse is within this item's X range
            if (mousePos.x >= bounds.min.x && mousePos.x <= bounds.max.x)
            {
                // Check top zone
                if (mousePos.y >= bounds.min.y && mousePos.y < bounds.min.y + insertZoneHeight)
                {
                    insertion.active = true;
                    insertion.insertParent = bounds.parent;
                    insertion.insertIndex = bounds.indexInParent;
                    insertion.lineStart = ImVec2(bounds.min.x, bounds.min.y);
                    insertion.lineEnd = ImVec2(bounds.max.x, bounds.min.y);
                    break;
                }
                // Check bottom zone
                else if (mousePos.y > bounds.max.y - insertZoneHeight && mousePos.y <= bounds.max.y)
                {
                    insertion.active = true;
                    insertion.insertParent = bounds.parent;
                    insertion.insertIndex = bounds.indexInParent + 1;
                    insertion.lineStart = ImVec2(bounds.min.x, bounds.max.y);
                    insertion.lineEnd = ImVec2(bounds.max.x, bounds.max.y);
                    break;
                }
            }
        }
    }

    // Draw insertion line and handle drop
    if (insertion.active)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddLine(insertion.lineStart, insertion.lineEnd, IM_COL32(255, 255, 0, 255), 3.0f);

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            // Get siblings at target level
            std::vector<entt::entity> targetSiblings;
            if (insertion.insertParent == entt::null)
            {
                targetSiblings = m_Scene->GetRootEntities();
            }
            else
            {
                targetSiblings = m_Scene->GetChildren(insertion.insertParent);
                std::sort(targetSiblings.begin(), targetSiblings.end(),
                    [this](entt::entity a, entt::entity b) {
                        return m_Scene->GetEntityOrder(a) < m_Scene->GetEntityOrder(b);
                    });
            }

            // Calculate target order
            size_t targetOrder = 0;
            if (insertion.insertIndex < targetSiblings.size())
            {
                targetOrder = m_Scene->GetEntityOrder(targetSiblings[insertion.insertIndex]);
            }
            else if (!targetSiblings.empty())
            {
                targetOrder = m_Scene->GetEntityOrder(targetSiblings.back()) + 1;
            }

            for (entt::entity draggedEnt : draggedEntities)
            {
                if (insertion.insertParent == entt::null)
                {
                    m_Scene->RemoveParent(draggedEnt);
                }
                else
                {
                    m_Scene->SetParent(draggedEnt, insertion.insertParent);
                }
                m_Scene->ReorderEntity(draggedEnt, targetOrder);
            }

            PLOG_INFO << "Moved and reordered " << draggedEntities.size() << " entities";
            insertion.active = false;
        }
    }

    // Empty space drop target for unparenting
    if (isDragging && !insertion.active)
    {
        ImGui::InvisibleButton("##EmptySpace", ImVec2(ImGui::GetContentRegionAvail().x, 50.0f));

        if (ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ENTITY");
            if (!payload)
            {
                payload = ImGui::AcceptDragDropPayload("DND_ENTITIES");
            }

            if (payload && payload->IsDelivery())
            {
                size_t count = payload->DataSize / sizeof(entt::entity);
                entt::entity* entities = (entt::entity*)payload->Data;

                for (size_t i = 0; i < count; i++)
                {
                    m_Scene->RemoveParent(entities[i]);
                }
                PLOG_INFO << "Unparented " << count << " entities";
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                IM_COL32(100, 100, 100, 50)
            );
        }
    }

    // Clear selection on empty click
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
    {
        ClearSelection();
    }

    // Reset drag state
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        isDragging = false;
        draggedEntity = entt::null;
        draggedEntities.clear();
        insertion.active = false;
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

		float fontSize = 16.0f * main_scale;
		io.Fonts->AddFontFromFileTTF("resources/fonts/Roboto-Regular.ttf", fontSize);

		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();

		// Rounding (Unreal has minimal rounding)
		style.WindowRounding = 0.0f;
		style.ChildRounding = 0.0f;
		style.FrameRounding = 0.0f;
		style.PopupRounding = 0.0f;
		style.ScrollbarRounding = 0.0f;
		style.GrabRounding = 0.0f;
		style.TabRounding = 0.0f;

		// Spacing
		style.WindowPadding = ImVec2(8.0f, 8.0f);
		style.FramePadding = ImVec2(4.0f, 3.0f);
		style.ItemSpacing = ImVec2(8.0f, 4.0f);
		style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
		style.IndentSpacing = 21.0f;
		style.ScrollbarSize = 14.0f;
		style.GrabMinSize = 10.0f;

		// Borders
		style.WindowBorderSize = 1.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupBorderSize = 1.0f;
		style.FrameBorderSize = 0.0f;
		style.TabBorderSize = 0.0f;

		// Colors - Unreal Engine Theme
		ImVec4* colors = style.Colors;

		// Unreal's signature colors
		ImVec4 unrealDark = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);        // Very dark background
		ImVec4 unrealMediumDark = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);  // Medium dark
		ImVec4 unrealMedium = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);      // Medium
		ImVec4 unrealLight = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);       // Lighter panels
		ImVec4 unrealAccent = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);      // Accent gray
		ImVec4 unrealHighlight = ImVec4(1.00f, 0.47f, 0.00f, 1.00f);   // Orange highlight
		ImVec4 unrealHighlightDim = ImVec4(0.80f, 0.38f, 0.00f, 1.00f); // Dimmer orange
		ImVec4 unrealBorder = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);      // Border color

		// Window
		colors[ImGuiCol_WindowBg] = unrealMedium;
		colors[ImGuiCol_ChildBg] = unrealMediumDark;
		colors[ImGuiCol_PopupBg] = unrealMedium;
		colors[ImGuiCol_Border] = unrealBorder;
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		// Title
		colors[ImGuiCol_TitleBg] = unrealDark;
		colors[ImGuiCol_TitleBgActive] = unrealDark;
		colors[ImGuiCol_TitleBgCollapsed] = unrealDark;

		// Menu
		colors[ImGuiCol_MenuBarBg] = unrealDark;

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg] = unrealMediumDark;
		colors[ImGuiCol_ScrollbarGrab] = unrealAccent;
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);

		// CheckMark & Sliders
		colors[ImGuiCol_CheckMark] = unrealHighlight;
		colors[ImGuiCol_SliderGrab] = unrealHighlightDim;
		colors[ImGuiCol_SliderGrabActive] = unrealHighlight;

		// Buttons
		colors[ImGuiCol_Button] = unrealLight;
		colors[ImGuiCol_ButtonHovered] = unrealAccent;
		colors[ImGuiCol_ButtonActive] = unrealHighlight;

		// Headers (collapsing headers, tree nodes)
		colors[ImGuiCol_Header] = unrealLight;
		colors[ImGuiCol_HeaderHovered] = unrealAccent;
		colors[ImGuiCol_HeaderActive] = unrealHighlight;

		// Separators
		colors[ImGuiCol_Separator] = unrealBorder;
		colors[ImGuiCol_SeparatorHovered] = unrealHighlightDim;
		colors[ImGuiCol_SeparatorActive] = unrealHighlight;

		// Resize Grip
		colors[ImGuiCol_ResizeGrip] = unrealLight;
		colors[ImGuiCol_ResizeGripHovered] = unrealAccent;
		colors[ImGuiCol_ResizeGripActive] = unrealHighlight;

		// Tabs
		colors[ImGuiCol_Tab] = unrealMediumDark;
		colors[ImGuiCol_TabHovered] = unrealAccent;
		colors[ImGuiCol_TabActive] = unrealLight;
		colors[ImGuiCol_TabUnfocused] = unrealMediumDark;
		colors[ImGuiCol_TabUnfocusedActive] = unrealMedium;

		// Docking
		colors[ImGuiCol_DockingPreview] = ImVec4(1.00f, 0.47f, 0.00f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = unrealDark;

		// Plots
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = unrealHighlight;
		colors[ImGuiCol_PlotHistogram] = unrealHighlightDim;
		colors[ImGuiCol_PlotHistogramHovered] = unrealHighlight;

		// Text
		colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.47f, 0.00f, 0.35f);

		// Inputs
		colors[ImGuiCol_FrameBg] = unrealMediumDark;
		colors[ImGuiCol_FrameBgHovered] = unrealLight;
		colors[ImGuiCol_FrameBgActive] = unrealAccent;

		// Drag and Drop
		colors[ImGuiCol_DragDropTarget] = unrealHighlight;

		// Nav highlight
		colors[ImGuiCol_NavHighlight] = unrealHighlight;
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);

		style.ScaleAllSizes(main_scale);

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

			ImGuiID dockLeft, dockCenterRight;
			ImGui::DockBuilderSplitNode(dockspaceId,ImGuiDir_Left, 0.25f,
				&dockLeft, &dockCenterRight);

			ImGuiID dockCenter, dockRight;
			ImGui::DockBuilderSplitNode(dockCenterRight, ImGuiDir_Right, 0.25f,
				&dockRight, &dockCenter);

			ImGui::DockBuilderDockWindow("Scene Hierarchy", dockLeft);
			ImGui::DockBuilderDockWindow("Scene", dockCenter);
			ImGui::DockBuilderDockWindow("Inspector", dockRight);

			ImGui::DockBuilderFinish(dockspaceId);
		}
	}

	void App::HandleEntityDeletion()
	{
		ImGuiIO& io = ImGui::GetIO();

		if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !io.WantTextInput)
		{
			if (m_HasSelection && !m_SelectedEntities.empty())
			{
				for (entt::entity entity: m_SelectedEntities)
				{
					if (m_Scene->IsValidEntity(entity))
					{
						m_Scene->DestroyEntityRecursive(entity);
					}
				}

				ClearSelection();
				PLOG_INFO << "Deleted selected entities";
			}
		}
	}

	void App::RenderInspector()
	{
		RenderInspectorMultiSelect();
	}

	void App::RenderInspectorMultiSelect()
	{
		ImGui::Begin("Inspector");

		if (!m_Scene)
		{
			ImGui::TextDisabled("No active scene");
			ImGui::End();
			return;
		}

		if (!m_HasSelection || m_SelectedEntities.empty())
		{
			ImGui::TextDisabled("No entity selected");
			ImGui::End();
			return;
		}

		entt::registry& registry = m_Scene->GetRegistry();

		if (m_SelectedEntities.size() == 1)
		{
			entt::entity entity = m_SelectedEntities[0];

			if (!m_Scene->IsValidEntity(entity))
			{
				ImGui::TextDisabled("Invalid entity");
				ImGui::End();
				return;
			}

			NameTag* nameTag = registry.try_get<NameTag>(entity);
			std::string entityName = nameTag? nameTag->name : "Unnamed Entity";
			auto entityID = static_cast<uint32_t>(entity);

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.47f, 0.0f, 1.0f));
			ImGui::Text("%s", entityName.c_str());
			ImGui::PopStyleColor();
			ImGui::Text("Entity ID: %u", entityID);
			ImGui::Separator();

			if (registry.all_of<NameTag>(entity))
			{
				auto& nameTag = registry.get<NameTag>(entity);
				MetaInspector::DrawComponentInspector(nameTag);
			}
			if (registry.all_of<Transform>(entity))
			{
				auto& transform = registry.get<Transform>(entity);
				MetaInspector::DrawComponentInspector(transform);
				transform.MarkDirty();
			}
			if (registry.all_of<MeshRenderer>(entity))
			{
				auto& meshRenderer = registry.get<MeshRenderer>(entity);

				if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Checkbox("Enabled", &meshRenderer.enabled);
					ImGui::Checkbox("Cast Shadows", &meshRenderer.castShadow);
					ImGui::Checkbox("Receive Shadows", &meshRenderer.receiveShadows);
					ImGui::Checkbox("Backface Culling", &meshRenderer.backfaceCulling);

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					if (ImGui::BeginCombo("Material",
						MaterialManager::GetInstance().GetMaterial(meshRenderer.materialId)?
						MaterialManager::GetInstance().GetMaterial(meshRenderer.materialId)->name.c_str()
							: "None"))
					{
						if (ImGui::Selectable("Default", meshRenderer.materialId == 1))
						{
							meshRenderer.materialId = 1;
							meshRenderer.ClearAllOverrides();
						}
						ImGui::EndCombo();
					}

					ImGui::Spacing();
				}

				if (ImGui::CollapsingHeader("Material Properties", ImGuiTreeNodeFlags_DefaultOpen))
				{
					MaterialPropertyInspector::Draw(meshRenderer);
				}
			}

			if (auto* meshFilter = registry.try_get<MeshFilter>(entity))
			{
				if (ImGui::CollapsingHeader("MeshFilter"))
				{
					ImGui::Text("Meshes: %zu", meshFilter->meshes.size());
					for (size_t i = 0; i < meshFilter->meshes.size(); i++)
					{
						const auto& mesh = meshFilter->meshes[i];
						if (ImGui::TreeNode((void*)(intptr_t)i, "Mesh %zu", i))
						{
							ImGui::Text("Vertices: %zu", mesh.vertices.size());
							ImGui::Text("Indices: %zu", mesh.indices.size());
							ImGui::Text("Triangles: %zu", mesh.indices.size() / 3);
							ImGui::TreePop();
						}
					}
					ImGui::Spacing();
				}
			}
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.47f, 0.0f, 1.0f));
			ImGui::Text("Multiple Entities Selected (%zu)", m_SelectedEntities.size());
			ImGui::PopStyleColor();
			ImGui::Separator();

			bool allHaveTransform = true;
			bool allHaveMeshRenderer = true;
			bool allHaveMeshFilter = true;

			for (entt::entity entity : m_SelectedEntities)
			{
				if (!registry.all_of<Transform>(entity)) allHaveTransform = false;
				if (!registry.all_of<MeshRenderer>(entity)) allHaveMeshRenderer = false;
				if (!registry.all_of<MeshFilter>(entity)) allHaveMeshFilter = false;

				if (registry.all_of<DirectionalLight>(entity))
				{
					auto& light = registry.get<DirectionalLight>(entity);

					if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::Checkbox("Enabled", &light.enabled);
						ImGui::ColorEdit3("Color", glm::value_ptr(light.color));
						ImGui::DragFloat("Intensity", &light.intensity, 0.1f, 0.0f, 10.0f);
						ImGui::Spacing();
					}
				}
			}

			if (allHaveTransform)
			{
				if (ImGui::CollapsingHeader("Transform (Common)"))
            {
                auto& firstTransform = registry.get<Transform>(m_SelectedEntities[0]);

                glm::vec3 commonPosition = firstTransform.position;
                glm::vec3 commonScale = firstTransform.scale;
                glm::vec3 commonRotation = glm::eulerAngles(firstTransform.rotation);

                bool positionSame = true, scaleSame = true, rotationSame = true;

                for (size_t i = 1; i < m_SelectedEntities.size(); i++)
                {
                    auto& transform = registry.get<Transform>(m_SelectedEntities[i]);
                    if (transform.position != commonPosition) positionSame = false;
                    if (transform.scale != commonScale) scaleSame = false;
                    if (glm::eulerAngles(transform.rotation) != commonRotation) rotationSame = false;
                }

                // Position
                ImGui::Text("Position:");
                if (positionSame)
                {
                    glm::vec3 newPos = commonPosition;
                    if (ImGui::DragFloat3("##Position", glm::value_ptr(newPos), 0.1f))
                    {
                        for (entt::entity entity : m_SelectedEntities)
                        {
                            auto& transform = registry.get<Transform>(entity);
                            transform.position = newPos;
                            transform.MarkDirty();
                        }
                    }
                }
                else
                {
                    ImGui::TextDisabled(" -");
                }

                // Scale
                ImGui::Text("Scale:");
                if (scaleSame)
                {
                    glm::vec3 newScale = commonScale;
                    if (ImGui::DragFloat3("##Scale", glm::value_ptr(newScale), 0.1f))
                    {
                        for (entt::entity entity : m_SelectedEntities)
                        {
                            auto& transform = registry.get<Transform>(entity);
                            transform.scale = newScale;
                            transform.MarkDirty();
                        }
                    }
                }
                else
                {
                    ImGui::TextDisabled(" -");
                }

                ImGui::Spacing();
            }
			}

			if (allHaveMeshRenderer)
			{
				ImGui::Text("MeshRenderer (Common)");
			}
			if (allHaveMeshFilter)
			{
				ImGui::Text("MeshFilter (Common)");
			}
		}

		ImGui::End();
	}

	bool App::IsEntitySelected(entt::entity entity) const
	{
		return std::find(m_SelectedEntities.begin(), m_SelectedEntities.end(), entity)
				!= m_SelectedEntities.end();
	}

	void App::SelectEntity(entt::entity entity, bool addToSelection)
	{
		if (!addToSelection)
		{
			m_SelectedEntities.clear();
		}

		if (!IsEntitySelected(entity))
		{
			m_SelectedEntities.push_back(entity);
		}

		m_HasSelection = !m_SelectedEntities.empty();
	}

	void App::DeselectEntity(entt::entity entity)
	{
		auto it = std::find(m_SelectedEntities.begin(), m_SelectedEntities.end(), entity);
		if (it != m_SelectedEntities.end())
		{
			m_SelectedEntities.erase(it);
		}
		m_HasSelection = !m_SelectedEntities.empty();
	}

	void App::ClearSelection()
	{
		m_SelectedEntities.clear();
		m_HasSelection = false;
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
