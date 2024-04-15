#pragma once
#include "nvrhi/nvrhi.h"

namespace Raito::Renderer::D3D12
{
	struct D3D12Callback final : nvrhi::IMessageCallback
	{
		D3D12Callback() = default;
		~D3D12Callback() override = default;

		void message(nvrhi::MessageSeverity severity, const char* messageText) override;
	};
}
