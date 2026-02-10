#pragma once
#include <unordered_map>
#include "Material.h"
#include "ShaderManager.h"
#include "shader/BlinnPhongShader.h"
#include "shader/UnlitShader.h"

namespace CPURDR
{
	using MaterialHandle = uint32_t;
	constexpr MaterialHandle INVALID_MATERIAL = 0;

	class MaterialManager
	{
	public:
		static MaterialManager& GetInstance()
		{
			static MaterialManager instance;
			return instance;
		}

		MaterialHandle CreateMaterial(const std::string& name, uint32_t shaderId)
		{
			MaterialHandle handle = m_NextHandle++;
			Material mat;
			mat.name = name;
			mat.shaderId = shaderId;

			IShader* shader = ShaderManager::GetInstance().GetShader(shaderId);
			if (shader)
			{
				for (const auto& prop : shader->GetProperties())
				{
					std::visit([&](auto&& val)
					{
						using T = std::decay_t<decltype(val)>;
						if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int> ||
									  std::is_same_v<T, glm::vec2> || std::is_same_v<T, glm::vec3> ||
									  std::is_same_v<T, glm::vec4> || std::is_same_v<T, uint32_t>)
						{
							mat.properties[prop.name] = val;
						}
					}, prop.defaultValue);
				}
			}

			m_Materials[handle] = std::move(mat);
			return handle;
		}

		Material* GetMaterial(MaterialHandle handle)
		{
			auto it = m_Materials.find(handle);
			return it != m_Materials.end()? &it->second:nullptr;
		}

		const Material* GetMaterial(MaterialHandle handle) const
		{
			auto it = m_Materials.find(handle);
			return it != m_Materials.end()? &it->second:nullptr;
		}

		MaterialHandle GetDefaultMaterial() const {return m_DefaultMaterial;}
		MaterialHandle GetDefaultPBRMaterial() const {return m_DefaultPBRMaterial;}
		MaterialHandle GetLightGizmoMaterial() const {return m_LightGizmoMaterial;}

		void Initialize()
		{
			m_DefaultMaterial = CreateMaterial("Default", BlinnPhongShader::Id);
			if (Material* mat = GetMaterial(m_DefaultMaterial))
			{
				mat->Set("_BaseColor", glm::vec3(0.8f));
				mat->Set("_SpecularColor", glm::vec3(1.0f));
				mat->Set("_Shininess", 32.0f);
			}

			m_DefaultPBRMaterial = CreateMaterial("Default PBR", PBRShader::Id);
			if (Material* mat = GetMaterial(m_DefaultPBRMaterial))
			{
				mat->Set("_Albedo", glm::vec3(0.8f));
				mat->Set("_Metallic", 0.0f);
				mat->Set("_Roughness", 0.5f);
				mat->Set("_AO", 1.0f);
			}

			m_LightGizmoMaterial = CreateMaterial("LightGizmo", UnlitShader::Id);
			if (Material* mat = GetMaterial(m_LightGizmoMaterial))
			{
				mat->Set("_Color", glm::vec3(1.0f, 0.9f, 0.4f));
			}
		}
	private:
		MaterialManager() = default;
		std::unordered_map<MaterialHandle, Material> m_Materials;
		MaterialHandle m_NextHandle = 1;
		MaterialHandle m_DefaultMaterial = INVALID_MATERIAL;
		MaterialHandle m_DefaultPBRMaterial = INVALID_MATERIAL;
		MaterialHandle m_LightGizmoMaterial = INVALID_MATERIAL;
	};
}