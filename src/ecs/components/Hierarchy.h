#pragma once
#include "entt.hpp"

namespace CPURDR
{
	struct Hierarchy
	{
		entt::entity parent = entt::null;
		std::vector<entt::entity> children;

		Hierarchy() = default;
		explicit Hierarchy(entt::entity parent) : parent(parent) {}

		bool HasParent() const {return parent != entt::null;}
		bool HasChildren() const {return !children.empty();}
		size_t GetChildren() const {return children.size();}

		void AddChild(entt::entity child)
		{
			if (std::find(children.begin(), children.end(), child) == children.end())
			{
				children.push_back(child);
			}
		}

		void RemoveChild(entt::entity child)
		{
			auto it = std::find(children.begin(), children.end(), child);
			if (it != children.end())
			{
				children.erase(it);
			}
		}
	};
}
