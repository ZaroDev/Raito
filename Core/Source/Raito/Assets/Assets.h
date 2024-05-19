#pragma once

#include "Mesh.h"
#include "Texture.h"

namespace Raito::Assets
{
	bool Initialize();

	u32 GetDefaultSphere();
	const Texture& GetChessTexture();
	u32 GetChessMaterial();
}
