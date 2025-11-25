#pragma once
#include <string>
#include "entt.hpp"

#include "ecs/components/MeshFilter.h"

namespace CPURDR
{
	class Scene
	{
	public:
		Scene(const std::string& name = "Untitled Scene");
		~Scene() = default;

		entt::entity CreateEntity(const std::string& name = "Entity");
		void DestroyEntity(entt::entity entity);
		bool IsValidEntity(entt::entity entity) const;

		entt::entity CreateMeshEntity(const std::string& name, const std::string& meshPath);
		entt::entity CreateMeshEntity(const std::string& name, const MeshFilter& meshFilter);

		entt::registry& GetRegistry(){return m_Registry;}
		const entt::registry& GetRegistry() const {return m_Registry;}

		const std::string& GetName() const {return m_Name;}
		void SetName(const std::string& name) {m_Name = name;}

		void SetParent(entt::entity child, entt::entity parent);
		void RemoveParent(entt::entity child);
		entt::entity GetParent(entt::entity child) const;
		std::vector<entt::entity> GetChildren(entt::entity entity) const;
		std::vector<entt::entity> GetRootEntities() const;

		void DestroyEntityRecursive(entt::entity entity);

		template<typename Func>
		void TraverseSceneGraph(Func&& func);

		template<typename Func>
		void TraverseFromEntity(entt::entity root, Func&& func);

		void Clear();
		size_t GetEntityCount() const;

		template<class... Components>
		auto View()
		{
			return m_Registry.view<Components...>();
		}

		template<class... Components>
		auto View() const
		{
			return m_Registry.view<Components...>();
		}
	private:
		void TraverseRecursive(entt::entity entity, const std::function<void(entt::entity)>& func);

	private:
		entt::registry m_Registry;
		std::string m_Name;
	};

	class SceneManager
	{
	public:
		static SceneManager& GetInstance();

		std::shared_ptr<Scene> CreateScene(const std::string& name = "New Scene");
		void SetActiveScene(std::shared_ptr<Scene> scene);
		std::shared_ptr<Scene> GetActiveScene() const {return m_ActiveScene;}

		void RemoveScene(const std::string& name);
		std::shared_ptr<Scene> GetScene(const std::string& name) const;
	private:
		SceneManager() = default;
		std::shared_ptr<Scene> m_ActiveScene;
		std::unordered_map<std::string, std::shared_ptr<Scene>> m_Scenes;
		static SceneManager* s_Instance;
	};
}
