#include "pch.h"
#include "Log.h"

#include <iostream>
#include <cstdarg>
#include <filesystem>
#include <sstream>

namespace  Raito::Core
{
	namespace
	{
		struct DebugStats
		{

		};
	}

	void Log(const std::string& title, const std::string& msg)
	{
		std::cout << "[Info]" << title << " - " << msg << std::endl;
	}
	void LogWarning(const std::string& title, const std::string& msg)
	{
		std::cout << "[Warn]" << title << " - " << msg << std::endl;
	}
	void LogError(const std::string& title, const std::string& msg)
	{
		std::cout << "[Error]" << title << " - " << msg << std::endl;
	}
}