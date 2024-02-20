#include "pch.h"
#include "Texture.h"

#include "Renderer/Renderer.h"


namespace Raito::Assets
{
	Texture::Texture(u32 width, u32 height, ubyte* data)
		: Width(width), Height(height)
	{
		Renderer::AddTexture(this, data);
	}

	Texture::~Texture()
	{
		Renderer::RemoveTexture(Id);
	}
}
