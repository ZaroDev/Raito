#pragma once
#include <Raito/Renderer/D3D12/D3D12Common.h>


namespace Raito::Renderer::D3D12
{
	class D3D12Surface
	{
	public:
		constexpr static uint32_t BufferCount = 3;
		constexpr static DXGI_FORMAT DefaultBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		D3D12Surface(SysWindow* window)
			: m_Window(window){}
		~D3D12Surface();

		void CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* commandQueue, DXGI_FORMAT format = DefaultBackBufferFormat);
		void Present() const;
		void Resize();

		constexpr u32 Height() const { return (u32)m_Viewport.Height; }
		constexpr u32 Width() const { return (u32)m_Viewport.Width; }
		constexpr const D3D12_VIEWPORT& Viewport() const { return m_Viewport; }
		constexpr const D3D12_RECT& ScissorRect() const { return m_ScissorRect; }
	private:
		ComPtr<IDXGISwapChain3> m_SwapChain = nullptr;
		DXGI_SWAP_CHAIN_DESC1   m_SwapChainDesc{};
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC m_FullScreenDesc{};

		D3D12_VIEWPORT m_Viewport{};
		D3D12_RECT m_ScissorRect{};
		u32 m_AllowTearing = 0;
		u32 m_PresentFlags = 0;

		bool m_TearingSupported = false;

		SysWindow* m_Window = nullptr;
	};
}