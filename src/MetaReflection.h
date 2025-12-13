#pragma once
#include <cfloat>
#include <entt.hpp>

namespace CPURDR
{
	using namespace entt::literals;

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
	};

	inline const FieldMetadata* GetFieldMetadata(const entt::meta_data& data)
	{
		return static_cast<const FieldMetadata*>(data.custom());
	}

	inline const ComponentMetadata* GetComponentMetadata(const entt::meta_type& type)
	{
		return static_cast<const ComponentMetadata*>(type.custom());
	}

	inline FieldMetadata GetFieldMetadataOrDefault(const entt::meta_data& data)
	{
		const FieldMetadata* meta = GetFieldMetadata(data);
		if (meta)
		{
			return *meta;
		}
		return FieldMetadata{};
	}

	inline ComponentMetadata GetComponentMetadataOrDefault(const entt::meta_type& type)
	{
		const ComponentMetadata* meta = GetComponentMetadata(type);
		if (meta)
		{
			return *meta;
		}
		return ComponentMetadata{};
	}
}