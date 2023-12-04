#pragma once
#include <string>
#include <format>

namespace Raito::Core
{
	inline static bool g_DumpLogs = true;

	void Log(const std::string& title, const std::string& msg);
	void LogWarning(const std::string& title, const std::string& msg);
	void LogError(const std::string& title, const std::string& msg);



}

#define FMT(...) std::format(__VA_ARGS__)
#ifndef DIST

#define LOG(x, ...)		Raito::Core::Log(x, FMT(__VA_ARGS__))
#define F_LOG(...)		Raito::Core::Log(__FUNCTION__, FMT(__VA__ARGS__))

#define WARN(x, ...)	Raito::Core::LogWarning(x, FMT(__VA_ARGS__))
#define F_WARN(...)		Raito::Core::LogWarning(__FUNCTION__, FMT(__VA_ARGS__))

#define ERR(x, ...)		Raito::Core::LogError(x, FMT(__VA_ARGS__))
#define F_ERR(...)		Raito::Core::LogError(__FUNCTION__, FMT(__VA_ARGS__))

#else

#define LOG(x, ...)	
#define F_LOG(...)	

#define WARN(x, ...)
#define F_WARN(...)	

#define ERR(x, ...)	
#define F_ERR(...)	

#endif