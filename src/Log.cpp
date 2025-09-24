#include "Log.h"

#include "plog/Init.h"
#include "plog/Formatters/TxtFormatter.h"
#include <plog/Appenders/ColorConsoleAppender.h>

namespace CPURDR
{
	class ColorFormatter
	{
	public:
		static plog::util::nstring format(const plog::Record& record)
		{
			// Get the base formatted message
			plog::util::nstring message = plog::TxtFormatter::format(record);

			// Add color codes based on severity
			plog::util::nstring colorCode;
			plog::util::nstring resetCode = PLOG_NSTR("\033[0m");

			switch (record.getSeverity())
			{
			case plog::fatal:
				colorCode = PLOG_NSTR("\033[97m\033[41m"); // white on red background
				break;
			case plog::error:
				colorCode = PLOG_NSTR("\033[91m"); // red
				break;
			case plog::warning:
				colorCode = PLOG_NSTR("\033[93m"); // yellow
				break;
			case plog::info:
				colorCode = PLOG_NSTR("\033[92m"); // green
				break;
			case plog::debug:
				colorCode = PLOG_NSTR("\033[96m"); // cyan
				break;
			case plog::verbose:
				colorCode = PLOG_NSTR("\033[90m"); // dark gray
				break;
			default:
				break;
			}

			return colorCode + message + resetCode;
		}
	};

	void Log::Init()
	{
		static plog::ColorConsoleAppender<ColorFormatter> consoleAppender;
		plog::init(plog::verbose, &consoleAppender);
	}


}
