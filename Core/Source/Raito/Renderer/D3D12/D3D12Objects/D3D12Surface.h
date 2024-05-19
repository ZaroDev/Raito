#pragma once
#include <Raito/Renderer/D3D12/D3D12Common.h>


namespace Raito::Renderer::D3D12
{
	class D3D12Surface
	{
	public:
		constexpr static uint32_t BufferCount = 3;
		constexpr static DXGI_FORMAT DefaultBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		D3D12Surface(SysWindow* window)
			: m_Window(window){}
		~D3D12Surface();

		void CreateSwapChain(IDXGIFactory2* factory, ID3D12CommandQueue* commandQueue, DXGI_FORMAT format = DefaultBackBufferFormat);
		void BeginFrame();
		void Present();
		void Resize();

		constexpr u32 Height() const { return (u32)m_Viewport.Height; }
		constexpr u32 Width() const { return (u32)m_Viewport.Width; }
		constexpr const D3D12_VIEWPORT& Viewport() const { return m_Viewport; }
		constexpr const D3D12_RECT& ScissorRect() const { return m_ScissorRect; }
	private:
		bool CreateRenderTargets();
		void ReleaseRenderTargets();
		void BackBufferResizing();
		void BackBufferResized();

		std::vector<nvrhi::FramebufferHandle> m_SwapChainFramebuffers;
		std::vector<nvrhi::TextureHandle>           m_RhiSwapChainBuffers;
		std::vector<nvrhi::RefCountPtr<ID3D12Resource>>    m_SwapChainBuffers;

		nvrhi::RefCountPtr<ID3D12CommandQueue>            m_GraphicsQueue;
		nvrhi::RefCountPtr<IDXGISwapChain3> m_SwapChain = nullptr;
		nvrhi::RefCountPtr<ID3D12Fence>     m_FrameFence;
		DXGI_SWAP_CHAIN_DESC1   m_SwapChainDesc{};
		std::vector<HANDLE>     m_FrameFenceEvents;
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC m_FullScreenDesc{};

		D3D12_VIEWPORT m_Viewport{};
		D3D12_RECT m_ScissorRect{};
		u32 m_AllowTearing = 0;
		u32 m_PresentFlags = 0;

		bool m_TearingSupported = false;

		u64 m_FrameCount = 1;
		SysWindow* m_Window = nullptr;
	};
}