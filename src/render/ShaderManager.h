#pragma once
#include "IShader.h"
#include "shader/BlinnPhongShader.h"
#include "shader/UnlitShader.h"
#include <memory>
#include <unordered_map>

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
			m_Shaders[BlinnPhongShader::Id] = std::make_unique<BlinnPhongShader>();
			m_Shaders[UnlitShader::Id] = std::make_unique<UnlitShader>();
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
	private:
		ShaderManager() = default;
		std::unordered_map<uint32_t, std::unique_ptr<IShader>> m_Shaders;
	};
}