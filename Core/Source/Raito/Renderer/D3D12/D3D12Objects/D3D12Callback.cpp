#include "pch.h"
#include "D3D12Callback.h"

#include "Renderer/D3D12/D3D12Common.h"

namespace Raito::Renderer::D3D12
{
	void D3D12Callback::message(nvrhi::MessageSeverity severity, const char* messageText)
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
}
