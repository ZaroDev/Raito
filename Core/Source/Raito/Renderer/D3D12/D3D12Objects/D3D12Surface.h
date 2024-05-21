#pragma once
#include <Raito/Renderer/D3D12/D3D12Common.h>


namespace Raito::Renderer::D3D12
{
	class D3D12Surface
	{
	public:
		constexpr static u32 s_BufferCount = 3;
		constexpr static  nvrhi::Format s_DefaultBackBufferFormat = nvrhi::Format::RGBA8_UNORM;

		explicit D3D12Surface(SysWindow* window)
			: m_Window(window) {}
		~D3D12Surface();

		DISABLE_COPY(D3D12Surface);

		D3D12Surface(D3D12Surface&& o)
			: m_SwapChain(o.m_SwapChain), m_Window(o.m_Window), m_CurrentBBIndex(o.m_CurrentBBIndex),
			m_Viewport(o.m_Viewport), m_ScissorRect(o.m_ScissorRect), m_AllowTearing(o.m_AllowTearing),
			m_PresentFlags(o.m_PresentFlags)
		{
			for (u32 i = 0; i < c_FrameBufferCount; i++)
			{
				m_RenderTargetData[i] = o.m_RenderTargetData[i];
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

		void CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* commandQueue, nvrhi::Format format = s_DefaultBackBufferFormat);
		void Present() const;
		void Resize();

		constexpr u32 Height() const { return (u32)m_Viewport.Height; }
		constexpr u32 Width() const { return (u32)m_Viewport.Width; }
		constexpr const D3D12_VIEWPORT& Viewport() const { return m_Viewport; }
		constexpr const D3D12_RECT& ScissorRect() const { return m_ScissorRect; }

		nvrhi::FramebufferHandle FrameBuffer(u32 id) { return m_FrameBuffer[id]; }



	private:
		void CreateSwapChainTextures();
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

		struct RenderTargetData
		{
			ID3D12Resource* Resource = nullptr;
			nvrhi::TextureHandle RTV{};
		};

		nvrhi::FramebufferHandle	m_FrameBuffer[s_BufferCount] = {};
		IDXGISwapChain4*			m_SwapChain = nullptr;
		RenderTargetData			m_RenderTargetData[s_BufferCount]{};
		D3D12_VIEWPORT				m_Viewport{};
		D3D12_RECT					m_ScissorRect{};
		u32							m_AllowTearing = 0;
		u32							m_PresentFlags = 0;
		SysWindow*					m_Window = nullptr;
		nvrhi::Format				m_Format{ s_DefaultBackBufferFormat };
		mutable u32					m_CurrentBBIndex = 0;
	};
}