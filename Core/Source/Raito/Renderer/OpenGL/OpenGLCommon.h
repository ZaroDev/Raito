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

#include "Renderer/Renderer.h"
#include "Core/Log.h"

#include <Core/Common.h>
#include <glad/glad.h>

#ifndef DIST
#define O_LOG(...) LOG("OpenGL", __VA_ARGS__)
#define O_WARN(...) WARN("OpenGL", __VA_ARGS__)
#define O_ERROR(...) ERR("OpenGL", __VA_ARGS__)
#else
#define O_LOG(...) 
#define O_WARN(...) 
#define O_ERROR(...) 
#endif

namespace Raito::Renderer::OpenGL
{
	struct TextureData
	{
		u64 Handle = 0;
		u32 Id = 0;
	};
}
