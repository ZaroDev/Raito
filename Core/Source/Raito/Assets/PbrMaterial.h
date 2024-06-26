#pragma once
#include "Texture.h"


namespace Raito::Assets
{
	struct PbrMaterial
	{
		Texture::TextureData Albedo;
		Texture::TextureData Normal;
		Texture::TextureData Emissive;
		Texture::TextureData MetalRoughness;
		Texture::TextureData AmbientOcclusion;
	};
}