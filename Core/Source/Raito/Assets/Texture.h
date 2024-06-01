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


namespace Raito::Assets
{

	//  TODO: Create texture import options distinct maps etc...
	//	For now all textures will have mip maps
	enum TextureType : u8
	{
		DIFFUSE,
		NORMAL,
		EMISSIVE,
		AMBIENT_OCCLUSION,
		METAL_ROUGHNESS,
		HDR
	};

	struct Texture final
	{
		Texture(u32 width, u32 height,  u32 nComponents, ubyte* data, TextureType type);
		~Texture();

		DEFAULT_MOVE_AND_COPY(Texture)

		u32 Id = 0;
		struct TextureData
		{
			u64 Handle = 0;
			u32 RenderId = 0;
		} RenderData;

		u32 Width = 0;
		u32 Height = 0;
		u32 ComponentsNum = 0;

		TextureType Type{};
	};
}