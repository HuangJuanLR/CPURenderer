#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>

#include "glm.hpp"

namespace CPURDR
{
	using TextureHandle = uint32_t;
	constexpr TextureHandle INVALID_TEXTURE = 0;

	using PropertyValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, TextureHandle>;

	struct Material
	{
		std::string name = "Default Material";
		uint32_t shaderId = 0;

		std::unordered_map<std::string, PropertyValue> properties;

		template<typename T>
		T Get(const std::string& propertyName, const T& defaultVal = T{}) const
		{
			auto it = properties.find(propertyName);
			if (it != properties.end())
			{
				if (auto* val = std::get_if<T>(&it->second))
					return *val;
			}
			return defaultVal;
		}

		template<typename T>
		void Set(const std::string& propertyName, const T& value)
		{
			properties[propertyName] = value;
		}

		float GetFloat(const std::string& propertyName, const float def = 0.0f) const
		{ return Get<float>(propertyName, def); }

		glm::vec3 GetVec3(const std::string& propertyName, const glm::vec3 def = glm::vec3(0)) const
		{ return Get<glm::vec3>(propertyName, def); }

		glm::vec4 GetVec4(const std::string& propertyName, const glm::vec4 def = glm::vec4(0)) const
		{ return Get<glm::vec4>(propertyName, def); }

		TextureHandle GetTexture(const std::string& propertyName) const
		{ return Get<TextureHandle>(propertyName, INVALID_TEXTURE); }
	};
}