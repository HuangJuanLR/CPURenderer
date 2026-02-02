#pragma once
#include "Material.h"
#include "../ecs/components/MeshRenderer.h"

namespace CPURDR
{
	inline Material CreateEffectiveMaterial(const Material& baseMaterial,
		const MeshRenderer& renderer)
	{
		Material effective;
		effective.name = baseMaterial.name + " (Instance)";
		effective.shaderId = baseMaterial.shaderId;

		effective.properties = baseMaterial.properties;

		// Can convert to [name, value] as structured bindings
		for (const auto& kvp: renderer.propertyOverrides)
		{
			effective.properties[kvp.first] = kvp.second;
		}

		return effective;
	}

	template<class T>
	T GetEffectiveProperty(
		const std::string& name,
		const Material& baseMaterial,
		const MeshRenderer& renderer,
		const T& defaultValue = T{})
	{
		if (renderer.HasOverride(name))
		{
			return renderer.GetOverride<T>(name, defaultValue);
		}
		return baseMaterial.Get<T>(name, defaultValue);
	}
}
