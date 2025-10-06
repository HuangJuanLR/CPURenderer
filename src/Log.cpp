#include "Log.h"
#include "plog/Init.h"
#include <plog/Appenders/ColorConsoleAppender.h>

namespace CPURDR
{
	void Log::Init()
	{
		static plog::ColorConsoleAppender<ColorFormatter> consoleAppender;
		plog::init(plog::verbose, &consoleAppender);
	}


}
