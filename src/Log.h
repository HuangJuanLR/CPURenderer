#pragma once
#include <memory>
#include "spdlog/spdlog.h"

namespace CPURDR
{
	class Log
	{
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetLogger() {return s_Logger;}
	private:
		static std::shared_ptr<spdlog::logger> s_Logger;

	};
}
