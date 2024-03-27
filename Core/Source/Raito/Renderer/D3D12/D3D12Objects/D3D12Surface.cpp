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

#include <pch.h>
#include "D3D12Surface.h"

#include <Raito/Renderer/D3D12/D3D12Core.h>

namespace Raito::Renderer::D3D12
{
	namespace
	{
		constexpr DXGI_FORMAT ToNonSRGB(DXGI_FORMAT format)
		{
			if (format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) return DXGI_FORMAT_R8G8B8A8_UNORM;

			return format;
		}
	}
	void D3D12Surface::CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* commandQueue, DXGI_FORMAT format)
	{
		ASSERT(factory && commandQueue);
		Release();

		if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_AllowTearing, sizeof(uint32_t)) && m_AllowTearing))
		{
			m_PresentFlags = DXGI_FEATURE_PRESENT_ALLOW_TEARING;
		}

		m_Format = format;

		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.BufferCount = BufferCount;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.Flags = m_AllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		desc.Format = ToNonSRGB(format);
		desc.Height = m_Window->Info.Height;
		desc.Width = m_Window->Info.Width;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Stereo = false;

		IDXGISwapChain1* swapChain;
		HWND hwnd = (HWND)m_Window->WindowHandle;
		DXCall(factory->CreateSwapChainForHwnd(commandQueue, hwnd, &desc, nullptr, nullptr, &swapChain))
			DXCall(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER))
			DXCall(swapChain->QueryInterface(IID_PPV_ARGS(&m_SwapChain)))
			Core::Release(swapChain);

		m_CurrentBBIndex = m_SwapChain->GetCurrentBackBufferIndex();

		for (uint32_t i = 0; i < BufferCount; i++)
		{
			m_RenderTargetData[i].RTV = D3D12MA::Allocator
		}
		Finalize();
	}

	void D3D12Surface::Present() const
	{
		ASSERT(m_SwapChain);
		DXCall(m_SwapChain->Present(0, m_PresentFlags))
			m_CurrentBBIndex = m_SwapChain->GetCurrentBackBufferIndex();
	}

	void D3D12Surface::Resize()
	{
		ASSERT(m_SwapChain);
		for (uint32_t i = 0; i < BufferCount; i++)
		{
			Core::Release(m_RenderTargetData[i].Resource);
		}
		const uint32_t flags = m_AllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		DXCall(m_SwapChain->ResizeBuffers(BufferCount, 0, 0, DXGI_FORMAT_UNKNOWN, flags));
		m_CurrentBBIndex = m_SwapChain->GetCurrentBackBufferIndex();

		Finalize();
	}

	void D3D12Surface::Finalize()
	{
		// Create RTVs for back-buffers
		for (uint32_t i = 0; i < BufferCount; i++)
		{
			RenderTargetData& data = m_RenderTargetData[i];
			ASSERT(!data.Resource);
			DXCall(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&data.Resource)))
				D3D12_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = m_Format;
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			Core::Device()->CreateRenderTargetView(data.Resource, &desc, data.RTV.Cpu);
		}

		DXGI_SWAP_CHAIN_DESC desc{};
		DXCall(m_SwapChain->GetDesc(&desc))
			const uint32_t width = desc.BufferDesc.Width;
		const uint32_t height = desc.BufferDesc.Height;
		D_LOG("Resized back buffer {0} Width, {1} Height", width, height);
		ASSERT(m_Window->Info.Width == width && m_Window->Info.Height == height);

		// Set viewport and scissor rect
		m_Viewport.TopLeftX = 0.f;
		m_Viewport.TopLeftY = 0.f;
		m_Viewport.Height = (float)height;
		m_Viewport.Width = (float)width;
		m_Viewport.MinDepth = 0.f;
		m_Viewport.MaxDepth = 1.f;

		m_ScissorRect = { 0, 0, (int32_t)width, (int32_t)height };
	}

	void D3D12Surface::Release()
	{
		for (uint32_t i = 0; i < BufferCount; i++)
		{
			RenderTargetData& data = m_RenderTargetData[i];
			Core::Release(data.Resource);
			//Core::RTVHeap().Free(data.RTV);
		}

		Core::Release(m_SwapChain);
	}
}