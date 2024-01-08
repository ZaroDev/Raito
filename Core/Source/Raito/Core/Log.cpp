#include "pch.h"
#include "Log.h"



#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Raito::Core::Debug
{
	namespace
	{
		static std::shared_ptr<spdlog::logger> s_Logger;
	}

	bool Initialize(bool logDumps)
	{
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		if (logDumps)
		{
			logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("raito.log", true));
			logSinks[1]->set_pattern("[%T] [%l] %n: %v");
		}

		s_Logger = std::make_shared<spdlog::logger>("Raito", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_Logger);
		s_Logger->set_level(spdlog::level::trace);
		s_Logger->flush_on(spdlog::level::trace);

		return true;
	}

	void Log(const std::string& title, const std::string& msg)
	{
		s_Logger->trace("{} - {}", title, msg);
	}
	void LogWarning(const std::string& title, const std::string& msg)
	{
		s_Logger->warn("{} - {}", title, msg);
	}
	void LogError(const std::string& title, const std::string& msg)
	{
		s_Logger->error("{} - {}", title, msg);
	}
}