#pragma once
#include <memory>
#include <unordered_map>

#include "IShader.h"
#include "shader/BlinnPhongShader.h"
#include "shader/UnlitShader.h"
#include "shader/PBRShader.h"

namespace CPURDR
{
	class ShaderManager
	{
	public:
		static ShaderManager& GetInstance()
		{
			static ShaderManager instance;
			return instance;
		}

		void Initialize()
		{
			RegisterShader(std::make_unique<BlinnPhongShader>());
			RegisterShader(std::make_unique<UnlitShader>());
			RegisterShader(std::make_unique<PBRShader>());
		}

		void RegisterShader(std::unique_ptr<IShader> shader)
		{
			uint32_t id = shader->GetId();
			m_ShaderList.push_back(shader.get());
			m_Shaders[id] = std::move(shader);
		}

		IShader* GetShader(uint32_t id) const
		{
			auto it = m_Shaders.find(id);
			return it != m_Shaders.end() ? it->second.get() : nullptr;
		}

		IShader* GetDefaultShader() const
		{
			return GetShader(BlinnPhongShader::Id);
		}

		const std::vector<IShader*>& GetAllShaders() const
		{
			return m_ShaderList;
		}

		bool IsInitialized() const
		{
			return !m_Shaders.empty();
		}
	private:
		ShaderManager() = default;
		std::unordered_map<uint32_t, std::unique_ptr<IShader>> m_Shaders;
		std::vector<IShader*> m_ShaderList;
	};
}
