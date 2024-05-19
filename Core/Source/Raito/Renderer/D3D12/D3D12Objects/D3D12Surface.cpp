#include "pch.h"
#include "D3D12Surface.h"

#include "Window/Window.h"
#include <Raito/Renderer/D3D12/D3D12Core.h>

#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

Raito::Renderer::D3D12::D3D12Surface::~D3D12Surface()
{
	m_RhiSwapChainBuffers.clear();
	ReleaseRenderTargets();

	for(auto fenceEvent : m_FrameFenceEvents)
	{
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
	}

	m_FrameFenceEvents.clear();

	m_FrameFenceEvents.clear();

	if (m_SwapChain)
	{
		m_SwapChain->SetFullscreenState(false, nullptr);
	}

	m_SwapChainBuffers.clear();
}

void Raito::Renderer::D3D12::D3D12Surface::CreateSwapChain(IDXGIFactory2* factory, ID3D12CommandQueue* commandQueue,
	DXGI_FORMAT format)
{
	ZeroMemory(&m_SwapChainDesc, sizeof(m_SwapChainDesc));
	m_SwapChainDesc.Width = m_Window->Info.Width;
	m_SwapChainDesc.Height = m_Window->Info.Height;
	m_SwapChainDesc.SampleDesc.Count = 1;
	m_SwapChainDesc.SampleDesc.Quality = 0;
	m_SwapChainDesc.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_SwapChainDesc.BufferCount = BufferCount;
	m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	m_SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	m_SwapChainDesc.Format = format;


	nvrhi::RefCountPtr<IDXGIFactory5> dxgiFactory5;
	if(SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&dxgiFactory5))))
	{
		BOOL supported = 0;
		if (SUCCEEDED(dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &supported, sizeof(supported)))) 
		{
			m_TearingSupported = (supported != 0);
		}
	}

	if(m_TearingSupported)
	{
		m_SwapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	}

	m_FullScreenDesc ={};
	m_FullScreenDesc.RefreshRate.Numerator = 0;
	m_FullScreenDesc.RefreshRate.Denominator = 1;
	m_FullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	m_FullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	m_FullScreenDesc.Windowed = true;
	HRESULT hr = E_FAIL;
	ComPtr<IDXGISwapChain1> pSwapChain1;
	m_GraphicsQueue = commandQueue;
	hr = factory->CreateSwapChainForHwnd(commandQueue, glfwGetWin32Window(static_cast<GLFWwindow*>(m_Window->Window)), &m_SwapChainDesc, &m_FullScreenDesc, nullptr, &pSwapChain1);
	if(FAILED(hr))
	{
		return;
	}
	hr = pSwapChain1->QueryInterface(IID_PPV_ARGS(&m_SwapChain));
	if(FAILED(hr))
	{
		return;
	}

	if (!CreateRenderTargets())
	{
		return;
	}
	
	hr = Core::Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_FrameFence));
	if (FAILED(hr))
	{
		return;
	}

	for (UINT bufferIndex = 0; bufferIndex < m_SwapChainDesc.BufferCount; bufferIndex++)
	{
		m_FrameFenceEvents.push_back(CreateEvent(nullptr, false, true, nullptr));
	}

}

void Raito::Renderer::D3D12::D3D12Surface::BeginFrame()
{
	DXGI_SWAP_CHAIN_DESC1 newSwapChainDesc;
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC newFullScreenDesc;
	if (SUCCEEDED(m_SwapChain->GetDesc1(&newSwapChainDesc)) && SUCCEEDED(m_SwapChain->GetFullscreenDesc(&newFullScreenDesc)))
	{
		if (m_FullScreenDesc.Windowed != newFullScreenDesc.Windowed)
		{
			BackBufferResizing();

			m_FullScreenDesc = newFullScreenDesc;
			m_SwapChainDesc = newSwapChainDesc;

			Resize();
			BackBufferResized();
		}

	}
}

void Raito::Renderer::D3D12::D3D12Surface::Present()
{
	auto bufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
	UINT presentFlags = 0;
	if (!m_Window->Info.VSync && m_FullScreenDesc.Windowed && m_TearingSupported)
		presentFlags |= DXGI_PRESENT_ALLOW_TEARING;

	m_SwapChain->Present(m_Window->Info.VSync ? 1 : 0, presentFlags);

	m_FrameFence->SetEventOnCompletion(m_FrameCount, m_FrameFenceEvents[bufferIndex]);
	m_GraphicsQueue->Signal(m_FrameFence, m_FrameCount);
	m_FrameCount++;
}

void Raito::Renderer::D3D12::D3D12Surface::Resize()
{
	ReleaseRenderTargets();

	if (!Core::NVDevice())
		return;

	if (!m_SwapChain)
		return;

	const HRESULT hr = m_SwapChain->ResizeBuffers(BufferCount,
		m_Window->Info.Width,
		m_Window->Info.Height,
		m_SwapChainDesc.Format,
		m_SwapChainDesc.Flags);

	bool ret = CreateRenderTargets();
}

bool Raito::Renderer::D3D12::D3D12Surface::CreateRenderTargets()
{
	m_SwapChainBuffers.resize(m_SwapChainDesc.BufferCount);
	m_RhiSwapChainBuffers.resize(m_SwapChainDesc.BufferCount);

	for(u32 i = 0; i < m_SwapChainDesc.BufferCount; i++)
	{
		const HRESULT hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffers[i]));
		if(FAILED(hr))
		{
			return false;
		}

		nvrhi::TextureDesc textureDesc;
		textureDesc.width = m_Window->Info.Width;
		textureDesc.height = m_Window->Info.Height;
		textureDesc.sampleCount = 1;
		textureDesc.sampleQuality = 0;
		textureDesc.format = nvrhi::Format::SRGBA8_UNORM;
		textureDesc.debugName = "SwapChainBuffer";
		textureDesc.isRenderTarget = true;
		textureDesc.isUAV = false;
		textureDesc.initialState = nvrhi::ResourceStates::Present;
		textureDesc.keepInitialState = true;
		m_RhiSwapChainBuffers[i] = Core::NVDevice()->createHandleForNativeTexture(nvrhi::ObjectTypes::D3D12_Resource, nvrhi::Object(m_SwapChainBuffers[i]), textureDesc);
	}

	return true;
}

void Raito::Renderer::D3D12::D3D12Surface::ReleaseRenderTargets()
{
	// Make sure that all frames have finished rendering
	Core::NVDevice()->waitForIdle();

	// Release all in-flight references to the render targets
	Core::NVDevice()->runGarbageCollection();

	// Set the events so that WaitForSingleObject in OneFrame will not hang later
	for (auto e : m_FrameFenceEvents)
		SetEvent(e);

	// Release the old buffers because ResizeBuffers requires that
	m_RhiSwapChainBuffers.clear();
	m_SwapChainBuffers.clear();
}

void Raito::Renderer::D3D12::D3D12Surface::BackBufferResizing()
{
	m_SwapChainFramebuffers.clear();
}

void Raito::Renderer::D3D12::D3D12Surface::BackBufferResized()
{

	uint32_t backBufferCount = BufferCount;
	m_SwapChainFramebuffers.resize(backBufferCount);
	for (uint32_t index = 0; index < backBufferCount; index++)
	{
		m_SwapChainFramebuffers[index] = Core::NVDevice()->createFramebuffer(
			nvrhi::FramebufferDesc().addColorAttachment(m_RhiSwapChainBuffers[index]));
	}
}
