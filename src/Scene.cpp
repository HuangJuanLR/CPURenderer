#include "Scene.h"

#include "ecs/components/MeshRenderer.h"
#include "ecs/components/NameTag.h"
#include "ecs/components/Transform.h"
#include "MeshLoader.h"
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
}
