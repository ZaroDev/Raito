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
#include "D3D12Command.h"

namespace Raito::Renderer::D3D12
{
	D3D12Command::D3D12Command(D3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
	{
		HRESULT hr = S_OK;

		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Type = type;
		DXCall(hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));

		if(FAILED(hr))
		{
			Release();
		}

		NAME_D3D12_OBJECT(m_CommandQueue,
			type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command Queue"
			: type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command Queue" : L"Command queue");

		for(u32 i = 0; i < c_FrameBufferCount; i++)
		{
			
		}
	}

	D3D12Command::~D3D12Command()
	{
	}

	void D3D12Command::Flush()
	{
	}

	void D3D12Command::Release()
	{
	}

	void D3D12Command::BeginFrame()
	{
	}

	void D3D12Command::EndFrame()
	{
	}

	void D3D12Command::CommandFrame::Wait(HANDLE fenceEvent, ID3D12Fence1* fence) const
	{
	}

	void D3D12Command::CommandFrame::Release()
	{
	}
}
