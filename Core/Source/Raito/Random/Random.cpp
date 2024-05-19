#include <pch.h>
#include "Random.h"

#include <Random>

namespace Raito::Random
{
	namespace
	{
		std::mt19937 g_RandomEngine;
		std::uniform_int_distribution<std::mt19937::result_type> g_Distribution;
	}


	void Initialize()
	{
		g_RandomEngine.seed(std::random_device()());
	}

	u32 UInt()
	{
		return g_Distribution(g_RandomEngine);
	}

	u32 UInt(u32 min, u32 max)
	{
		return min + (g_Distribution(g_RandomEngine) % (max - min + 1));
	}

	float Float()
	{
		return (float)g_Distribution(g_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
	}

	float Float(float min, float max)
	{
		return Float()* (max - min) + min;
	}

	V3 Vec3()
	{
		return V3(Float(), Float(), Float());
	}

	V3 Vec3(float min, float max)
	{
		return V3(Float() * (max - min) + min, Float() * (max - min) + min, Float() * (max - min) + min);
	}

	V3 InUnitSphere()
	{
		return glm::normalize(Vec3(-1.0f, 1.0f));
	}
}
