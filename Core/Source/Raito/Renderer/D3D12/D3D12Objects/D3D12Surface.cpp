#include "pch.h"
#include "D3D12Surface.h"

#include "Window/Window.h"

Raito::Renderer::D3D12::D3D12Surface::~D3D12Surface()
{
}

void Raito::Renderer::D3D12::D3D12Surface::CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* commandQueue,
	DXGI_FORMAT format)
{
	ZeroMemory(&m_SwapChainDesc, sizeof(m_SwapChainDesc));
	m_SwapChainDesc.Width = m_Window->Info.Width;
	m_SwapChainDesc.Height = m_Window->Info.Height;
	m_SwapChainDesc.SampleDesc.Count = 1;
	m_SwapChainDesc.SampleDesc.Quality = 0;
	m_SwapChainDesc.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_SwapChainDesc.BufferCount = BufferCount;
	m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	m_SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	m_SwapChainDesc.Format = format;


	ComPtr<IDXGIFactory5> dxgiFactory5;
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



}

void Raito::Renderer::D3D12::D3D12Surface::Present() const
{
}

void Raito::Renderer::D3D12::D3D12Surface::Resize()
{
}
