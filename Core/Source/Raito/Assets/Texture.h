#pragma once


namespace Raito::Assets
{

	//  TODO: Create texture import options distinct maps etc...
	//	For now all textures will have mip maps
	struct Texture final
	{
		Texture(u32 width, u32 height,  ubyte* data);
		~Texture();

		DEFAULT_MOVE_AND_COPY(Texture)

		u32 Id = 0;
		u32 Width = 0;
		u32 Height = 0;
	};
}