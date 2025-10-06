#pragma once
#include "plog/Log.h"
#include "plog/Formatters/TxtFormatter.h"


namespace CPURDR
{
	class ColorFormatter
	{
	public:
		static plog::util::nstring format(const plog::Record& record)
		{
			plog::util::nstring message = plog::TxtFormatter::format(record);

			plog::util::nstring colorCode;
			plog::util::nstring resetCode = PLOG_NSTR("\x1B[0m\x1B[0K");

			switch (record.getSeverity())
			{
			case plog::fatal:
				colorCode = PLOG_NSTR("\x1B[97m\x1B[41m");
				break;
			case plog::error:
				colorCode = PLOG_NSTR("\x1B[91m");
				break;
			case plog::warning:
				colorCode = PLOG_NSTR("\x1B[93m");
				break;
			case plog::info:
				// default color
				break;
			case plog::debug:
			case plog::verbose:
				colorCode = PLOG_NSTR("\x1B[96m");
				break;
			default:
				break;
			}

			return colorCode + message + resetCode;
		}
	};

	class Log
	{
	public:
		static void Init();
	};
}
