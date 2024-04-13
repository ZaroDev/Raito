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

#include "pch.h"
#include "D3D12Core.h"
#include "D3D12Common.h"

#ifdef DEBUG
#include <nvrhi/validation.h>
#endif

#define HR_RETURN(hr, msg) if(FAILED(hr)) { D_LOG(msg); return false; }
namespace Raito::Renderer::D3D12::Core
{
	using namespace Microsoft::WRL;

	namespace
	{
		struct D3D12Callback : public nvrhi::IMessageCallback
		{
			D3D12Callback() = default;
			virtual ~D3D12Callback() = default;

			void message(nvrhi::MessageSeverity severity, const char* messageText) override
			{
				switch (severity)
				{
				case nvrhi::MessageSeverity::Info: D_LOG("{0}", messageText);
					break;
				case nvrhi::MessageSeverity::Warning: D_WARN("{0}", messageText);
					break;
				case nvrhi::MessageSeverity::Error: D_ERROR("{0}", messageText);
					break;
				case nvrhi::MessageSeverity::Fatal: D_ERROR("{0}", messageText);
					break;
				}
			}
		};

		constexpr D3D_FEATURE_LEVEL c_MinimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;

		ComPtr<IDXGIFactory2> g_DXGIFactory = nullptr;
		ComPtr<IDXGISwapChain3> g_SwapChain = nullptr;
		ComPtr<IDXGIAdapter> g_DXGIAdapter = nullptr;
		ComPtr<D3D12Device> g_Device = nullptr;
		ComPtr<ID3D12CommandQueue> g_GraphicsCommandQueue = nullptr;
		ComPtr<ID3D12CommandQueue> g_ComputeCommandQueue = nullptr;
		ComPtr<ID3D12CommandQueue> g_CopyCommandQueue = nullptr;

		nvrhi::DeviceHandle g_DeviceHandle;

		D3D12Callback g_Callback{};


		D3D_FEATURE_LEVEL GetMaxFeatureLevel(IDXGIAdapter* adapter)
		{
			constexpr D3D_FEATURE_LEVEL featureLevels[4] =
			{
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_12_1
			};

			D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelInfo{};
			featureLevelInfo.NumFeatureLevels = _countof(featureLevels);
			featureLevelInfo.pFeatureLevelsRequested = featureLevels;
			ComPtr<ID3D12Device> device;
			DXCall(D3D12CreateDevice(adapter, c_MinimumFeatureLevel, IID_PPV_ARGS(&device)));
			DXCall(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevelInfo, sizeof(featureLevelInfo)));
			return featureLevelInfo.MaxSupportedFeatureLevel;
		}
	}

	
	


	bool Initialize()
	{
		HRESULT hr;
		u32 dxgiFactoryFlags = NULL;

#ifdef DEBUG
		ComPtr<ID3D12Debug> debug;
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
		HR_RETURN(hr, "Failed to create debug interface");
		debug->EnableDebugLayer();
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

		// Create the factory
		DXCall(hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&g_DXGIFactory)));
		if (!g_DXGIFactory)
		{
			D_ERROR("Failed to create DXGI Factory!");
			return false;
		}


		if(FAILED(g_DXGIFactory->EnumAdapters(0, &g_DXGIAdapter)))
		{
			D_ERROR("Cannot find any DXGI adapters in the system");
			return false;
		}


		// Determine what is the maximum feature leve that is supported.
		const D3D_FEATURE_LEVEL maxFeatureLevel = GetMaxFeatureLevel(g_DXGIAdapter.Get());
		ASSERT(maxFeatureLevel >= c_MinimumFeatureLevel);
		if (maxFeatureLevel < c_MinimumFeatureLevel)
		{
			D_ERROR("Maximum feature level is lower than minimum feature level!");
			return false;
		}

		DXCall(hr = D3D12CreateDevice(g_DXGIAdapter.Get(), maxFeatureLevel, IID_PPV_ARGS(&g_Device)));
	

		// Create all the command queues
		D3D12_COMMAND_QUEUE_DESC queueDesc;
		ZeroMemory(&queueDesc, sizeof(queueDesc));
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.NodeMask = 1;

		// Graphics queue
		DXCall(hr = g_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_GraphicsCommandQueue)));
		HR_RETURN(hr, "Failed to create graphics queue");
		g_GraphicsCommandQueue->SetName(L"Graphics Queue");

		// Compute queue
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		DXCall(hr = g_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_ComputeCommandQueue)));
		HR_RETURN(hr, "Failed to create compute queue");
		g_ComputeCommandQueue->SetName(L"Compute Queue");

		// Copy queue
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		DXCall(hr = g_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_CopyCommandQueue)));
		HR_RETURN(hr, "Failed to create copy queue");
		g_ComputeCommandQueue->SetName(L"Copy Queue");


		nvrhi::d3d12::DeviceDesc deviceDesc;
		deviceDesc.errorCB = &g_Callback;
		deviceDesc.pDevice = g_Device.Get();
		deviceDesc.pGraphicsCommandQueue = g_GraphicsCommandQueue.Get();
		deviceDesc.pComputeCommandQueue = g_ComputeCommandQueue.Get();
		deviceDesc.pCopyCommandQueue = g_CopyCommandQueue.Get();

		g_DeviceHandle = nvrhi::d3d12::createDevice(deviceDesc);

#ifdef DEBUG
		const nvrhi::DeviceHandle validationLayer = nvrhi::validation::createValidationLayer(g_DeviceHandle);
		g_DeviceHandle = validationLayer;
#endif

		return true;
	}

	void Shutdown()
	{
		
	}

	void SetDeferredReleasesFlags()
	{
	}

	ComPtr<D3D12Device> Device()
	{
		return g_Device;
	}

	Surface CreateSurface(SysWindow* window)
	{
		return Surface();
	}

	void RemoveSurface(u32 id)
	{
	}

	void ResizeSurface(u32 id, u32 width, u32 height)
	{
	}

	u32 SurfaceWidth(u32 id)
	{
		return 0;
	}

	u32 SurfaceHeight(u32 id)
	{
		return 0;
	}

	u32 GetColorGetAttachment(u32 target, u32 id)
	{
		return 0;
	}

	u32 GetDepthAttachment(u32 id)
	{
		return 0;
	}

	void RenderSurface(u32 id)
	{
	}


	void DeferredRelease(IUnknown* resource)
	{
	
	}

	u32 CurrentFrameIndex()
	{
		return 0;
	}
}
