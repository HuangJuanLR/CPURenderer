#include "Log.h"

#include "plog/Formatters/TxtFormatter.h"

namespace CPURDR
{
	void Log::Init()
	{
		plog::init<plog::TxtFormatter>(plog::debug, plog::streamStdOut);
	}


}
