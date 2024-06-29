#pragma once

#include "Mesh.h"
#include "Texture.h"

namespace Raito::Assets
{
	bool Initialize();

	u32 GetDefaultSphere();
	const Texture& GetChessTexture();
	const Texture& GetWhiteTexture();
	const Texture& GetBlackTexture();
	u32 GetChessMaterial();
}
