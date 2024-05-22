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
#include "D3D12ShaderCompilation.h"
#include "D3D12Objects/D3D12Callback.h"
#include "D3D12Objects/D3D12Command.h"
#include "D3D12Objects/D3D12Surface.h"

#include "D3D12Shaders.h"
#include "D3D12Passes/D3D12DeferredPass.h"
#include "nvrhi/utils.h"
#include "Time/Time.h"

#ifdef DEBUG
#define ENABLE_VALIDATION 
#include <dxgidebug.h>
#include <nvrhi/validation.h>
#endif

#define HR_RETURN(hr, msg) if(FAILED(hr)) { D_LOG(msg); return false; }


namespace Raito::Renderer::D3D12::Core
{
	using namespace Microsoft::WRL;

	namespace
	{
		Camera g_Camera(45.0, 0.1f, 1000.0f);

		std::vector<D3D12Surface> g_Surfaces;

		constexpr D3D_FEATURE_LEVEL c_MinimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;

		nvrhi::RefCountPtr<IDXGIFactory7> g_DXGIFactory = nullptr;
		nvrhi::RefCountPtr<IDXGIAdapter> g_DXGIAdapter = nullptr;
		nvrhi::RefCountPtr<D3D12Device> g_Device = nullptr;
		D3D12Command g_GraphicsCommandQueue;
		D3D12Command g_ComputeCommandQueue;
		D3D12Command g_CopyCommandQueue;

		nvrhi::DeviceHandle g_DeviceHandle;

		D3D12Callback g_Callback{};


		std::mutex g_DeferredReleaseMutex;
		std::vector<IUnknown*> g_DeferredReleases[c_FrameBufferCount]{};
		u32 g_DeferredReleasesFlags[c_FrameBufferCount]{};

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


	void __declspec(noinline) ProcessDeferredReleases(u32 frameIndex)
	{
		std::lock_guard lock{ g_DeferredReleaseMutex };
		// NOTE: We clear this flag in the beginning. If we'd clear it at the end
		// then it might overwrite some other thread that was trying to see it.
		// it's fine it overwriting happens before processing items.
		g_DeferredReleasesFlags[frameIndex] = 0;

		std::vector<IUnknown*>& resources = g_DeferredReleases[frameIndex];
		if (!resources.empty())
		{
			for (auto& resource : resources)
			{
				Release(resource);
			}
			resources.clear();
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


		if (FAILED(g_DXGIFactory->EnumAdapters(0, &g_DXGIAdapter)))
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

		new (&g_GraphicsCommandQueue) D3D12Command(g_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		new (&g_ComputeCommandQueue) D3D12Command(g_Device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
		new (&g_CopyCommandQueue) D3D12Command(g_Device, D3D12_COMMAND_LIST_TYPE_COPY);



		nvrhi::d3d12::DeviceDesc deviceDesc;
		deviceDesc.errorCB = &g_Callback;
		deviceDesc.pDevice = g_Device.Get();
		deviceDesc.pGraphicsCommandQueue = g_GraphicsCommandQueue.CommandQueue();
		deviceDesc.pComputeCommandQueue = g_ComputeCommandQueue.CommandQueue();
		deviceDesc.pCopyCommandQueue = g_CopyCommandQueue.CommandQueue();

		g_DeviceHandle = nvrhi::d3d12::createDevice(deviceDesc);

		g_GraphicsCommandQueue.CreateCommandList();
		g_ComputeCommandQueue.CreateCommandList();
		g_CopyCommandQueue.CreateCommandList();

#ifdef ENABLE_VALIDATION
		const nvrhi::DeviceHandle validationLayer = nvrhi::validation::createValidationLayer(g_DeviceHandle);
		g_DeviceHandle = validationLayer;
#endif

		D3D12_MESSAGE_ID disableMessageIDs[] = {
			   D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
			   D3D12_MESSAGE_ID_COMMAND_LIST_STATIC_DESCRIPTOR_RESOURCE_DIMENSION_MISMATCH, // descriptor validation doesn't understand acceleration structures
		};

		nvrhi::RefCountPtr<ID3D12InfoQueue> pInfoQueue;
		g_Device->QueryInterface(&pInfoQueue);
		if (pInfoQueue)
		{
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.pIDList = disableMessageIDs;
			filter.DenyList.NumIDs = sizeof(disableMessageIDs) / sizeof(disableMessageIDs[0]);
			pInfoQueue->AddStorageFilterEntries(&filter);
		}


		Shaders::CompileShaders();
		Shaders::Initialize();

		Deferred::Initialize();

		

		return true;
	}

	void Shutdown()
	{
		g_GraphicsCommandQueue.Release();
		g_ComputeCommandQueue.Release();
		g_CopyCommandQueue.Release();

		// NOTE: We don't call ProcessDeferredReleases at the end because
		// some resources (such as swap chains) can't be released before
		// their depending on resources are released
		for (uint32_t i = 0; i < c_FrameBufferCount; i++)
		{
			ProcessDeferredReleases(i);
		}
		g_DXGIFactory = nullptr;

		// NOTE: Some types only use deferred release for their resources during
		// shutdown/reset/clear. To finally release these resources we call
		// ProcessDeferredReleases once more.
		ProcessDeferredReleases(0);

		Deferred::Shutdown();


		g_Surfaces.clear();

		g_DeviceHandle = nullptr;
#ifdef DEBUG
		{
			{
				ComPtr<ID3D12InfoQueue> infoQueue;
				DXCall(g_Device->QueryInterface(IID_PPV_ARGS(&infoQueue)))
					infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			}
			ComPtr<ID3D12DebugDevice2> debugDevice;
			DXCall(g_Device->QueryInterface(IID_PPV_ARGS(&debugDevice)))

				g_Device = nullptr;
			DXCall(debugDevice->ReportLiveDeviceObjects(
				D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL))
		}
#else
		g_Device = nullptr;
#endif
	}

	void SetDeferredReleasesFlags()
	{
		g_DeferredReleasesFlags[CurrentFrameIndex()] = 1;
	}

	nvrhi::RefCountPtr<D3D12Device> Device()
	{
		return g_Device;
	}

	nvrhi::DeviceHandle NVDevice()
	{
		return g_DeviceHandle;
	}

	Surface CreateSurface(SysWindow* window)
	{
		D3D12Surface& surface = g_Surfaces.emplace_back(window);
		surface.CreateSwapChain(g_DXGIFactory, g_GraphicsCommandQueue.CommandQueue());

		return Surface(g_Surfaces.size() - 1);
	}

	void RemoveSurface(u32 id)
	{
		g_GraphicsCommandQueue.Flush();
		g_Surfaces.erase(g_Surfaces.begin() + id);
	}

	const D3D12Command& CopyCommand()
	{
		return g_CopyCommandQueue;
	}

	const D3D12Command& GraphicsCommand()
	{
		return g_GraphicsCommandQueue;
	}

	void ResizeSurface(u32 id, u32 width, u32 height)
	{
		g_GraphicsCommandQueue.Flush();
		g_Surfaces[id].Resize();
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
		g_GraphicsCommandQueue.BeginFrame();
		const nvrhi::CommandListHandle& commandList = g_GraphicsCommandQueue.CommandList();
		const uint32_t frameIndex = CurrentFrameIndex();

		if (g_DeferredReleasesFlags[frameIndex])
		{
			ProcessDeferredReleases(frameIndex);
		}
		D3D12Surface& surface = g_Surfaces[id];

		g_Camera.OnResize(surface.Width(), surface.Height());
		g_Camera.OnUpdate(Time::GetDeltaTime() / 1000.0f);

		Deferred::Update(&g_Camera, surface, frameIndex);


		g_GraphicsCommandQueue.EndFrame(surface);
	}


	void DeferredRelease(IUnknown* resource)
	{
		const u32 frameIndex = CurrentFrameIndex();
		std::lock_guard lock{ g_DeferredReleaseMutex };
		g_DeferredReleases[frameIndex].push_back(resource);
		SetDeferredReleasesFlags();
	}

	u32 CurrentFrameIndex()
	{
		return g_GraphicsCommandQueue.FrameIndex();
	}
}
