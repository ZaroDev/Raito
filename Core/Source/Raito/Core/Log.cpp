#include "pch.h"
#include "Log.h"

#include <iostream>
#include <cstdarg>
#include <filesystem>
#include <sstream>
#include <fstream>

namespace Raito::Core::Debug
{
	namespace
	{
		std::stringstream Buffer{};
		struct DebugStats
		{
			std::filesystem::path SinkPath = "Logs/log-dump.txt";

			DebugStats() = default;
			~DebugStats()
			{
				if (g_DumpLogs)
				{
					if (!std::filesystem::exists("Logs"))
						std::filesystem::create_directory("Logs");

					std::ofstream dumpLog(SinkPath);

					dumpLog.write(Buffer.str().c_str(), Buffer.str().size());
					
					dumpLog.close();
				}
			}
		} DebugStats{};
	}

	void Log(const std::string& title, const std::string& msg)
	{
		Buffer << "[Info]" << title << " - " << msg << std::endl;
	}
	void LogWarning(const std::string& title, const std::string& msg)
	{
		Buffer << "[Warning]" << title << " - " << msg << std::endl;
	}
	void LogError(const std::string& title, const std::string& msg)
	{
		Buffer << "[Error]" << title << " - " << msg << std::endl;
	}
}