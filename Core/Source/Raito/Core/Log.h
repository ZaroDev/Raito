/*
MIT License

Copyright (c) 2023 Víctor Falcón Zaro

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <string>
#include <format>

namespace Raito::Core::Debug
{
	inline static bool g_DumpLogs = true; /**< Logging flag. Enables/Disables log dumping to a file when the program ends */

	//! Information loggin function
	void Log(const std::string& title, const std::string& msg);
	//! Warning logging function
	void LogWarning(const std::string& title, const std::string& msg);
	//! Error logging function
	void LogError(const std::string& title, const std::string& msg);
}
//! Formating macro
#define FMT(...) std::format(__VA_ARGS__)

#ifndef DIST

	//! Information logging macro where the title is specified
	//! @param x Title for the log
	//! @params ...Variable arguments to log as a message
	#define LOG(x, ...)		Raito::Core::Debug::Log(x, FMT(__VA_ARGS__))
	//! Information logging macro where the title is the function caller name
	//! @params ...Variable arguments to log as a message
	#define F_LOG(...)		Raito::Core::Debug::Log(__FUNCTION__, FMT(__VA__ARGS__))
	
	//! Warning logging macro where the title is specified
	//! @param x Title for the log
	//! @params ...Variable arguments to log as a message
	#define WARN(x, ...)	Raito::Core::Debug::LogWarning(x, FMT(__VA_ARGS__))
	//! Warning logging macro where the title is the function caller name
	//! @params ...Variable arguments to log as a message
	#define F_WARN(...)		Raito::Core::Debug::LogWarning(__FUNCTION__, FMT(__VA_ARGS__))
	
	//! Error logging macro where the title is specified
	//! @param x Title for the log
	//! @params ...Variable arguments to log as a message
	#define ERR(x, ...)		Raito::Core::Debug::LogError(x, FMT(__VA_ARGS__))
	//! Error logging macro where the title is the function caller name
	//! @params ...Variable arguments to log as a message
	#define F_ERR(...)		Raito::Core::Debug::LogError(__FUNCTION__, FMT(__VA_ARGS__))

#else

	#define LOG(x, ...)	
	#define F_LOG(...)	
	
	#define WARN(x, ...)
	#define F_WARN(...)	
	
	#define ERR(x, ...)	
	#define F_ERR(...)	

#endif