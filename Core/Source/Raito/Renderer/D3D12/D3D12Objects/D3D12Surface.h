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

#include <Raito/Renderer/D3D12/D3D12Common.h>
#include <Raito/Renderer/D3D12/D3D12Objects/D3D12Resources.h>

#include "Window/Window.h"

namespace Raito::Renderer::D3D12
{
	class D3D12Surface
	{
	public:
		constexpr static uint32_t BufferCount = 3;
		constexpr static DXGI_FORMAT DefaultBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		explicit D3D12Surface(SysWindow* window)
			: m_Window(window)
		{
			ASSERT(window->WindowHandle);
		}
		~D3D12Surface()
		{
			Release();
		}

#if USE_STL_VECTOR
		DISABLE_COPY(D3D12Surface)

			constexpr D3D12Surface(D3D12Surface&& o)
			: m_SwapChain(o.m_SwapChain), m_Window(o.m_Window), m_CurrentBBIndex(o.m_CurrentBBIndex),
			m_Viewport(o.m_Viewport), m_ScissorRect(o.m_ScissorRect), m_AllowTearing(o.m_AllowTearing),
			m_PresentFlags(o.m_PresentFlags)
		{
			for (u32 i = 0; i < c_FrameBufferCount; i++)
			{
				m_RenderTargetData[i].Resource = o.m_RenderTargetData->Resource;
				m_RenderTargetData[i].RTV = o.m_RenderTargetData->RTV;
			}
			o.Reset();
		}

		constexpr D3D12Surface& operator=(D3D12Surface&& o)
		{
			ASSERT(this != &o);
			if (this != &o)
			{
				Release();
				Move(o);
			}
			return *this;
		}
#endif

		void CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* commandQueue, DXGI_FORMAT format = DefaultBackBufferFormat);
		void Present() const;
		void Resize();

		constexpr u32 Height() const { return (u32)m_Viewport.Height; }
		constexpr u32 Width() const { return (u32)m_Viewport.Width; }
		constexpr ID3D12Resource* const BackBuffer() const { return m_RenderTargetData[m_CurrentBBIndex].Resource; }
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE RTV() const { return m_RenderTargetData[m_CurrentBBIndex].RTV.Cpu; }
		constexpr const D3D12_VIEWPORT& Viewport() const { return m_Viewport; }
		constexpr const D3D12_RECT& ScissorRect() const { return m_ScissorRect; }
	private:
		void Finalize();
		void Release();

		constexpr void Move(D3D12Surface& o)
		{
			m_SwapChain = o.m_SwapChain;
			for (u32 i = 0; i < c_FrameBufferCount; i++)
			{
				m_RenderTargetData[i] = o.m_RenderTargetData[i];
			}
			m_Viewport = o.m_Viewport;
			m_ScissorRect = o.m_ScissorRect;
			m_Window = o.m_Window;
			m_CurrentBBIndex = o.m_CurrentBBIndex;
			m_AllowTearing = o.m_AllowTearing;
			m_PresentFlags = o.m_PresentFlags;
			o.Reset();
		}

		constexpr void Reset()
		{
			m_SwapChain = nullptr;
			for (u32 i = 0; i < c_FrameBufferCount; i++)
			{
				m_RenderTargetData[i] = {};
			}
			m_Viewport = {};
			m_ScissorRect = {};
			m_Window = {};
			m_CurrentBBIndex = 0;
			m_AllowTearing = 0;
			m_PresentFlags = 0;
		}

	private:
		struct RenderTargetData
		{
			ID3D12Resource* Resource = nullptr;
			DescriptorHandle RTV{};
		};

		IDXGISwapChain4* m_SwapChain = nullptr;
		RenderTargetData                m_RenderTargetData[BufferCount]{};
		D3D12_VIEWPORT                  m_Viewport{};
		D3D12_RECT                      m_ScissorRect{};
		u32								m_AllowTearing = 0;
		u32								m_PresentFlags = 0;
		SysWindow* m_Window = nullptr;
		DXGI_FORMAT                     m_Format{ DefaultBackBufferFormat };
		mutable u32                m_CurrentBBIndex = 0;
	};
}
