#include "pch.h"
#include "Time.h"

namespace Raito::Time
{
	namespace
	{
		float g_DeltaTime;

		std::chrono::time_point<std::chrono::steady_clock> g_LastTime;

		float g_TimeSinceStart = 0.0f;
	}

	float GetTimeSinceStart()
	{
		return g_TimeSinceStart;
	}
	float GetDeltaTime()
	{
		return g_DeltaTime;
	}

	float GetFPS()
	{
		return 1000.0f / g_DeltaTime;
	}

	void StartTimeUpdate()
	{
		g_LastTime = std::chrono::high_resolution_clock::now();
	}

	void EndTimeUpdate()
	{
		const auto endTime = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<float> duration = (endTime - g_LastTime);
		g_DeltaTime = duration.count() * 1000.0f;

		g_TimeSinceStart += duration.count();
	}
}
