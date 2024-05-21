#include "pch.h"
#include "D3D12Surface.h"

#include "Window/Window.h"
#include <Raito/Renderer/D3D12/D3D12Core.h>

#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include "optick/include/optick.h"

Raito::Renderer::D3D12::D3D12Surface::~D3D12Surface()
{
	Release();
}

void Raito::Renderer::D3D12::D3D12Surface::CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* commandQueue,
	nvrhi::Format format)
{
	ASSERT(factory && commandQueue);

	Release();

	BOOL supported = 0;
	if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &supported, sizeof(supported))))
	{
		m_PresentFlags = DXGI_FEATURE_PRESENT_ALLOW_TEARING;
	}

	m_Format = format;

	DXGI_SWAP_CHAIN_DESC1 desc{};
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = m_Window->Info.Width;
	desc.Height = m_Window->Info.Height;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = s_BufferCount;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.Flags = m_AllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	desc.Format = nvrhi::d3d12::convertFormat(m_Format);

	IDXGISwapChain1* swapChain;

	HWND hwnd = static_cast<HWND>(m_Window->WindowHandle);
	DXCall(factory->CreateSwapChainForHwnd(commandQueue, hwnd, &desc, nullptr, nullptr, &swapChain));
	DXCall(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
	DXCall(swapChain->QueryInterface(IID_PPV_ARGS(&m_SwapChain)));

	Core::Release(swapChain);

	m_CurrentBBIndex = m_SwapChain->GetCurrentBackBufferIndex();

	Finalize();
}


void Raito::Renderer::D3D12::D3D12Surface::Present() const
{
	OPTICK_EVENT();

	ASSERT(m_SwapChain);
	DXCall(m_SwapChain->Present(0, m_PresentFlags));
	m_CurrentBBIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void Raito::Renderer::D3D12::D3D12Surface::Resize()
{
	ASSERT(m_SwapChain);
	for (uint32_t i = 0; i < s_BufferCount; i++)
	{
		Core::Release(m_RenderTargetData[i].Resource);
	}
	const uint32_t flags = m_AllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	DXCall(m_SwapChain->ResizeBuffers(s_BufferCount, 0, 0, DXGI_FORMAT_UNKNOWN, flags));
	m_CurrentBBIndex = m_SwapChain->GetCurrentBackBufferIndex();

	Finalize();
}

void Raito::Renderer::D3D12::D3D12Surface::CreateSwapChainTextures()
{
	for(u32 i = 0; i < s_BufferCount; i++)
	{
		RenderTargetData& data = m_RenderTargetData[i];
		DXCall(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&data.Resource)));
		auto textureDesc = nvrhi::TextureDesc()
			.setDimension(nvrhi::TextureDimension::Texture2D)
			.setFormat(m_Format)
			.setWidth(m_Window->Info.Width)
			.setHeight(m_Window->Info.Height)
			.setIsRenderTarget(true)
			.setDebugName("Swap chain texture");

		data.RTV = Core::NVDevice()->createHandleForNativeTexture(nvrhi::ObjectTypes::D3D12_Resource,
			nvrhi::Object(data.Resource),
			textureDesc);

		auto frameBufferDesc = nvrhi::FramebufferDesc()
			.addColorAttachment(data.RTV);

		m_FrameBuffer[i] = Core::NVDevice()->createFramebuffer(frameBufferDesc);
	}
}

void Raito::Renderer::D3D12::D3D12Surface::Finalize()
{
	CreateSwapChainTextures();

	DXGI_SWAP_CHAIN_DESC desc{};
	DXCall(m_SwapChain->GetDesc(&desc));
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

void Raito::Renderer::D3D12::D3D12Surface::Release()
{
	for(u32 i = 0; i < s_BufferCount; i++)
	{
		
	}
	Core::Release(m_SwapChain);
}
