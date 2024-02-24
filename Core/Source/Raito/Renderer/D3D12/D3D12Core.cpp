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

namespace Raito::Renderer::D3D12::Core
{
	using namespace Microsoft::WRL;

	namespace
	{
		D3D12Device* g_MainDevice = nullptr;
		D3D12Factory* g_Factory = nullptr;

		constexpr D3D_FEATURE_LEVEL c_MinimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;

		std::vector<IUnknown*> g_DeferredReleases[c_FrameBufferCount]{};
		u32 g_DeferredReleasesFlags[c_FrameBufferCount]{};
		std::mutex g_DeferredReleasesMutex{};
	}

	// Get the first most performing adapter with the minimum feature level
	IDXGIAdapter4* DetermineAdapter()
	{
		IDXGIAdapter4* adapter = nullptr;

		// Get the adapters in descending order of performance
		for (u32 i = 0;
			g_Factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
			i++)
		{
			if (SUCCEEDED(D3D12CreateDevice(adapter, c_MinimumFeatureLevel, __uuidof(ID3D12Device), nullptr)))
			{
				return adapter;
			}
			Release(adapter);
		}
		D_ERROR("Couldn't find any suitable main adapter!");
		return nullptr;
	}

	D3D_FEATURE_LEVEL GetMaxFeatureLevel(IDXGIAdapter4* adapter)
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

	bool Failed()
	{

		Shutdown();
		return false;
	}


	bool Initialize()
	{
		if (g_MainDevice)
		{
			D_WARN("Main device was already created reseting rendererer");
			Shutdown();
		}


		u32 dxgiFactoryFlags = NULL;

#ifdef	DEBUG
		// Enable debugging layer. Requires "Graphics Tools" optional feature
		{
			ComPtr<ID3D12Debug3> debugInterface;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
			{
				debugInterface->EnableDebugLayer();
#if ENABLE_GPU_BASE_VALIDATION
				debugInterface->SetEnableGPUBasedValidation(1);
#endif
			}
			else
			{
				D_WARN("Debug layer interface is not available. Verify that Graphics Tools optional feature is intalled in this system!");
			}
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
#endif

		// Create main factory
		HRESULT hr = S_OK;
		DXCall(hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&g_Factory)));
		if (!g_Factory)
		{
			D_ERROR("Failed to create DXGI Factory!");
			return Failed();
		}

		// Determine the physical device to use
		ComPtr<IDXGIAdapter4> mainAdapter;
		mainAdapter.Attach(DetermineAdapter());
		if (!mainAdapter)
		{
			D_ERROR("Failed to create main adapter");
			return Failed();
		}

		// Determine what is the maximum feature leve that is supported.
		const D3D_FEATURE_LEVEL maxFeatureLevel = GetMaxFeatureLevel(mainAdapter.Get());
		ASSERT(maxFeatureLevel >= c_MinimumFeatureLevel);
		if (maxFeatureLevel < c_MinimumFeatureLevel)
		{
			D_ERROR("Maximum feature level is lower than minimum feature level!");
			return Failed();
		}
		// Create the logical device
		DXCall(hr = D3D12CreateDevice(mainAdapter.Get(), maxFeatureLevel, IID_PPV_ARGS(&g_MainDevice)));
		if (!g_MainDevice)
		{
			D_ERROR("Couldn't create a device with the maximum feature level!");
			return Failed();
		}

		NAME_D3D12_OBJECT(g_MainDevice, L"Main D3D12 Device");

#ifdef DEBUG
		{
			ComPtr<ID3D12InfoQueue> infoQueue;
			DXCall(g_MainDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)));
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		}
#endif // DEBUG


		return true;
	}

	void Shutdown()
	{
		D_LOG("Shutting down renderer");

		Release(g_Factory);

#ifdef DEBUG
		{
			{
				ComPtr<ID3D12InfoQueue> infoQueue;
				DXCall(g_MainDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)));
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			}
			ComPtr<ID3D12DebugDevice2> debugDevice;
			DXCall(g_MainDevice->QueryInterface(IID_PPV_ARGS(&debugDevice)));
			Release(g_MainDevice);
			DXCall(debugDevice->ReportLiveDeviceObjects(
				D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL));
		}
#else
		Release(g_MainDevice);
#endif
	}

	void SetDeferredReleasesFlags()
	{
		g_DeferredReleasesFlags[CurrentFrameIndex()] = 1;
	}


	void DeferredRelease(IUnknown* resource)
	{
		const u32 frameIndex = CurrentFrameIndex();

		std::lock_guard lock{g_DeferredReleasesMutex};
		g_DeferredReleases[frameIndex].push_back(resource);

		SetDeferredReleasesFlags();
	}

	u32 CurrentFrameIndex()
	{
		return 0;
	}
}
