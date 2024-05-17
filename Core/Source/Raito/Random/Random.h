#pragma once
#include "Math/MathTypes.h"


namespace Raito::Random
{
	void Initialize();

	NODISCARD u32 UInt();
	NODISCARD u32 UInt(u32 min, u32 max);
	NODISCARD float Float();
	NODISCARD V3 Vec3();
	NODISCARD V3 Vec3(float min, float max);
	NODISCARD V3 InUnitSphere();
}
