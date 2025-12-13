#pragma once
#include <float.h>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace CPURDR
{
	struct FieldMetadata
	{
		std::string displayName;
		bool editable = true;
		bool visible = true;
		float speed = 0.01f;
		float minValue = -FLT_MAX;
		float maxValue = FLT_MAX;
		bool isColor = false;

		bool convertQuaternionToEuler = false;
	};

	struct ComponentMetadata
	{
		std::string displayName;
		bool visibleInInspector = true;
		bool defaultOpen = true;
		std::unordered_map<size_t, FieldMetadata> fieldMetadata; // field index as key, field metadata as value
	};

	class MetadataRegistry
	{
	public:
		static MetadataRegistry& Instance()
		{
			static MetadataRegistry instance;
			return instance;
		}

		template<typename Component>
		void RegisterComponent(const ComponentMetadata& metadata)
		{
			m_ComponentMetadata[std::type_index(typeid(Component))] = metadata;
		}

		template<typename Component>
		ComponentMetadata* GetComponentMetadata()
		{
			auto it = m_ComponentMetadata.find(std::type_index(typeid(Component)));
			return (it != m_ComponentMetadata.end())? &it->second : nullptr;
		}
	private:
		std::unordered_map<std::type_index, ComponentMetadata> m_ComponentMetadata;
	};
}
