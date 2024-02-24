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
#include "Raito/Core/BasicTypes.h"
#include "Raito/Core/Assert.h"
#include "Raito/Renderer/Renderer.h"

#include <dxgi1_6.h>
#include <d3d12.h>

// Contains ComPtr (a.k.a Windows smart ptr)
#include <wrl.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

namespace Raito::Renderer::D3D12
{
	constexpr u32 c_FrameBufferCount = 3;
	using D3D12Device = ID3D12Device8;
	using D3D12GraphicsCommandList = ID3D12GraphicsCommandList6;
    using D3D12Factory = IDXGIFactory7;
}

#define ENABLE_GPU_BASE_VALIDATION 0

#ifndef DIST
#define D_LOG(...) LOG("D3D12", __VA_ARGS__)
#define D_WARN(...) WARN("D3D12", __VA_ARGS__)
#define D_ERROR(...) ERR("D3D12", __VA_ARGS__)
#else
#define D_LOG(x) 
#define D_WARN(x) 
#define D_ERROR(x) 
#endif


#ifdef DEBUG
#ifndef DXCall
#define DXCall(x)                          \
        if(FAILED(x)) {                        \
            ASSERT(false);              \
        }                                      
#endif
#else
#define DXCall(x) x;
#endif

#ifdef DEBUG
#define NAME_D3D12_OBJECT(obj, name) obj->SetName(name); \
        {                                                    \
            std::wstring wstr(name);                         \
            std::string str(wstr.begin(), wstr.end());       \
            D_LOG("Object created: {}", str);   \
        }
#define NAME_D3D12_OBJECT_INDEXED(obj, n, name)             \
    {                                                           \
        wchar_t fullName[128];                                  \
        if(swprintf_s(fullName, L"%s[%u]", name, n) > 0) {      \
            obj->SetName(fullName);                             \
            std::wstring wstr(fullName);                        \
            std::string str(wstr.begin(), wstr.end());          \
            D_LOG("Object created: {}", str);     \
        }                                                       \
    }
#else
#define NAME_D3D12_OBJECT(x, name)
#define NAME_D3D12_OBJECT_INDEXED(obj, n, name)
#endif

