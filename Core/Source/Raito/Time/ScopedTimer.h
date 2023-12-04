#pragma once
#include <chrono>


namespace Raito
{
	class ScopedTimer
	{
	public:
		ScopedTimer(const char* name);
		~ScopedTimer();

	private:
		std::string m_Name{};
		std::chrono::time_point<std::chrono::steady_clock> m_Start{}, m_End{};
		std::chrono::duration<float> m_Duration{};
	};
}

