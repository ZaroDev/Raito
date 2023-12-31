#include "pch.h"
#include "Renderer.h"
#include "RHI.h"

#include "D3D12/D3D12RHI.h"

namespace Raito::Renderer
{
	namespace
	{
		RHI GraphicsContext{};
		API GraphicsAPI = API::NONE;

		bool SetPlatformInterface()
		{
			switch (GraphicsAPI)
			{
			case API::NONE: ASSERT(false); return false;
			case API::D3D12: D3D12::GetPlatformInterface(GraphicsContext); break;
			case API::VULKAN: ASSERT(false); return false;
			case API::OPENGL: ASSERT(false); return false;
			}
			return true;
		}
	}

	bool Initialize(API api)
	{
		ASSERT(api != API::NONE && api != API::COUNT);
		LOG("Renderer", "Initializing Renderer");
		GraphicsAPI = api;
		return SetPlatformInterface() && GraphicsContext.Initialize();
	}
	void Shutdown()
	{
		GraphicsContext.Shutdown();
	}
}