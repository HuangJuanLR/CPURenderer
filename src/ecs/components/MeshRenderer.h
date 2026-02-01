#pragma once
#include <cstdint>
#include <variant>

namespace CPURDR
{
	using PropertyValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, uint32_t>;

	struct MeshRenderer
	{
		bool enabled = true;
		bool castShadow = true;
		bool receiveShadows = true;
		bool backfaceCulling = true;

		uint32_t materialId = 1;

		std::unordered_map<std::string, PropertyValue> propertyOverrides;

		bool HasOverride(const std::string& name) const
		{
			return propertyOverrides.contains(name);
		}

		template<typename T>
		void SetOverride(const std::string& name, const T& value)
		{
			propertyOverrides[name] = value;
		}

		template<typename T>
		T GetOverride(const std::string& name, const T& defaultValue = T{}) const
		{
			auto it = propertyOverrides.find(name);
			if (it != propertyOverrides.end())
			{
				if (auto* val = std::get_if<T>(&it->second))
					return *val;
			}
			return defaultValue;
		}

		void ClearOverride(const std::string& name)
		{
			propertyOverrides.erase(name);
		}

		void ClearAllOverrides()
		{
			propertyOverrides.clear();
		}
	};
}
