#pragma once
#include <string>

namespace CPURDR
{
	struct NameTag
	{
		std::string name;

		NameTag() = default;
		explicit NameTag(const std::string& n): name(n){}
		explicit NameTag(std::string&& n): name(std::move(n)){}
	};
}
