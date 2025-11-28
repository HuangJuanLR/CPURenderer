#include "Scene.h"

#include "ecs/components/MeshRenderer.h"
#include "ecs/components/NameTag.h"
#include "ecs/components/Transform.h"
#include "MeshLoader.h"
#include "ecs/components/Hierarchy.h"
#include "plog/Log.h"

namespace CPURDR
{
	Scene::Scene(const std::string& name):m_Name(name)
	{
		PLOG_INFO << "Created scene" << name;
	}

	entt::entity Scene::CreateEntity(const std::string& name)
	{
		entt::entity entity = m_Registry.create();
		m_Registry.emplace<NameTag>(entity, name);
		m_Registry.emplace<Transform>(entity);
		m_Registry.emplace<Hierarchy>(entity);
		return entity;
	}

	void Scene::DestroyEntity(entt::entity entity)
	{
		if (m_Registry.valid(entity))
		{
			m_Registry.destroy(entity);
		}
	}

	bool Scene::IsValidEntity(entt::entity entity) const
	{
		return m_Registry.valid(entity);
	}

	entt::entity Scene::CreateMeshEntity(const std::string& name, const std::string& meshPath)
	{
		entt::entity entity = CreateEntity(name);

		auto& loader = MeshLoader::GetInstance();
		std::vector<Mesh> meshes = loader.LoadMeshFromFile(meshPath);

		m_Registry.emplace<MeshFilter>(entity, std::move(meshes));
		m_Registry.emplace<MeshRenderer>(entity);

		return entity;
	}

	entt::entity Scene::CreateMeshEntity(const std::string& name, const MeshFilter& meshFilter)
	{
		entt::entity entity = CreateEntity(name);
		m_Registry.emplace<MeshFilter>(entity, meshFilter);
		m_Registry.emplace<MeshRenderer>(entity);
		return entity;
	}

	void Scene::Clear()
	{
		m_Registry.clear();
		PLOG_INFO << "Cleared scene" << m_Name;
	}

	size_t Scene::GetEntityCount() const
	{
		return m_Registry.storage<entt::entity>()->size();
	}


	// ==============================
	// SceneManager
	// ==============================
	SceneManager* SceneManager::s_Instance = nullptr;

	SceneManager& SceneManager::GetInstance()
	{
		if (s_Instance == nullptr)
		{
			s_Instance = new SceneManager();
		}
		return *s_Instance;
	}

	std::shared_ptr<Scene> SceneManager::CreateScene(const std::string& name)
	{
		auto scene = std::make_shared<Scene>(name);
		m_Scenes[name] = scene;

		if (!m_ActiveScene)
		{
			m_ActiveScene = scene;
		}
		return scene;
	}

	void SceneManager::SetActiveScene(std::shared_ptr<Scene> scene)
	{
		m_ActiveScene = scene;
		PLOG_INFO << "Active scene set to:" << scene->GetName();
	}

	void SceneManager::RemoveScene(const std::string& name)
	{
		auto it = m_Scenes.find(name);
		if (it != m_Scenes.end())
		{
			if (m_ActiveScene == it->second)
			{
				m_ActiveScene = nullptr;
			}
			m_Scenes.erase(it);
		}
	}

	std::shared_ptr<Scene> SceneManager::GetScene(const std::string& name) const
	{
		auto it = m_Scenes.find(name);
		return it != m_Scenes.end()? it->second : nullptr;
	}

	void Scene::SetParent(entt::entity child, entt::entity parent)
	{
		if (!IsValidEntity(child) || !IsValidEntity(parent))
		{
			PLOG_ERROR << "Cannot set parent: invalid entity";
			return;
		}

		if (child == parent)
		{
			PLOG_ERROR << "Cannot set entity as its own parent";
			return;
		}

		auto* childHierarchy = m_Registry.try_get<Hierarchy>(child);
		if (childHierarchy && childHierarchy->HasParent())
		{
			RemoveParent(child);
		}

		if (!childHierarchy)
		{
			childHierarchy = &m_Registry.emplace<Hierarchy>(child);
		}

		auto& parentHierarchy = m_Registry.get_or_emplace<Hierarchy>(parent);

		childHierarchy->parent = parent;
		parentHierarchy.AddChild(child);

		if (auto* transform = m_Registry.try_get<Transform>(child))
		{
			transform->MarkDirty();
		}

		PLOG_DEBUG << "Set parent relationship: child=" << (uint32_t)child << ", parent=" << (uint32_t)parent;
	}

	void Scene::RemoveParent(entt::entity child)
	{
		if (!IsValidEntity(child)) return;

		auto* hierarchy = m_Registry.try_get<Hierarchy>(child);
		if (!hierarchy || !hierarchy->HasParent())
		{
			return;
		}

		entt::entity parent = hierarchy->parent;

		if (IsValidEntity(parent))
		{
			if (auto* parentHierarchy = m_Registry.try_get<Hierarchy>(parent))
			{
				parentHierarchy->RemoveChild(child);
			}
		}

		hierarchy->parent = entt::null;

		if (auto* transform = m_Registry.try_get<Transform>(child))
		{
			transform->MarkDirty();
		}
	}

	entt::entity Scene::GetParent(entt::entity entity) const
	{
		if (!IsValidEntity(entity)) return entt::null;

		auto* hierarchy = m_Registry.try_get<Hierarchy>(entity);
		return hierarchy? hierarchy->parent : entt::null;
	}

	std::vector<entt::entity> Scene::GetChildren(entt::entity entity) const
	{
		if (!IsValidEntity(entity)) return {};

		auto* hierarchy = m_Registry.try_get<Hierarchy>(entity);
		return hierarchy? hierarchy->children : std::vector<entt::entity>();
	}

	std::vector<entt::entity> Scene::GetRootEntities() const
	{
		std::vector<entt::entity> roots;

		auto view = m_Registry.view<Transform>();
		for (auto entity : view)
		{
			auto* hierarchy = m_Registry.try_get<Hierarchy>(entity);
			if (hierarchy && !hierarchy->HasParent())
			{
				roots.push_back(entity);
			}
		}

		return roots;
	}

	void Scene::DestroyEntityRecursive(entt::entity entity)
	{
		if (!IsValidEntity(entity)) return;

		auto children = GetChildren(entity);
		for (entt::entity child: children)
		{
			DestroyEntityRecursive(child);
		}

		RemoveParent(entity);
		DestroyEntity(entity);
	}

	void Scene::TraverseRecursive(entt::entity entity, const std::function<void(entt::entity)>& func)
	{
		if (!IsValidEntity(entity)) return;

		func(entity);

		auto* hierarchy = m_Registry.try_get<Hierarchy>(entity);
		if (hierarchy &&  hierarchy->HasChildren())
		{
			for (entt::entity child: hierarchy->children)
			{
				TraverseRecursive(child, func);
			}
		}
	}

	template <typename Func>
	void Scene::TraverseSceneGraph(Func&& func)
	{
		auto roots = GetRootEntities();
		for (entt::entity root: roots)
		{
			TraverseRecursive(root, std::forward<Func>(func));
		}
	}

	template <typename Func>
	void Scene::TraverseFromEntity(entt::entity root, Func&& func)
	{
		TraverseRecursive(root, std::forward<Func>(func));
	}


}
