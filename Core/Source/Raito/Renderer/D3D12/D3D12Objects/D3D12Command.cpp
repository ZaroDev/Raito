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

#include "Renderer/D3D12/D3D12Core.h"

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

		if (FAILED(hr))
		{
			Release();
		}

		NAME_D3D12_OBJECT(m_CommandQueue,
			type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command Queue"
			: type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command Queue" : L"Command queue");

		for (u32 i = 0; i < c_FrameBufferCount; i++)
		{
			CommandFrame& frame = m_CmdFrames[i];
			DXCall(hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&frame.CmdAllocator)));
			if (FAILED(hr))
			{
				Release();
			}

			NAME_D3D12_OBJECT_INDEXED(frame.CmdAllocator, i,
				type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command Queue"
				: type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
				L"Compute Command Queue" : L"Command queue");
		}

		DXCall(hr = device->CreateCommandList(0, type, m_CmdFrames[0].CmdAllocator, nullptr, IID_PPV_ARGS(&m_CommandList)));
		if (FAILED(hr))
		{
			Release();
		}

		DXCall(m_CommandList->Close());
		NAME_D3D12_OBJECT(m_CommandList,
			type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
			L"GFX Command list" :
			type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
			L"Compute command list" : L"Command list");

		DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));

		if (FAILED(hr))
		{
			Release();
		}

		m_FenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		ASSERT(m_FenceEvent);
		if (!m_FenceEvent)
		{
			Release();
		}

	}

	D3D12Command::~D3D12Command()
	{
		ASSERT(!m_CommandQueue && !m_CommandList && !m_Fence);
	}

	void D3D12Command::Flush()
	{
		for (u32 i = 0; i < c_FrameBufferCount; i++)
		{
			m_CmdFrames[i].Wait(m_FenceEvent, m_Fence);
		}
		m_FrameIndex = 0;
	}

	void D3D12Command::Release()
	{
		Flush();
		Core::Release(m_Fence);
		m_FenceValue = 0;

		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;

		Core::Release(m_CommandQueue);
		Core::Release(m_CommandList);

		for (u32 i = 0; i < c_FrameBufferCount; i++)
		{
			m_CmdFrames[i].Release();
		}
	}

	void D3D12Command::BeginFrame()
	{
		CommandFrame& frame = m_CmdFrames[m_FrameIndex];
		frame.Wait(m_FenceEvent, m_Fence);

		DXCall(frame.CmdAllocator->Reset());
		DXCall(m_CommandList->Reset(frame.CmdAllocator, nullptr));
	}

	void D3D12Command::EndFrame(const D3D12Surface& surface)
	{
		DXCall(m_CommandList->Close());
		ID3D12CommandList* const cmdLists[] = { m_CommandList };
		m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), &cmdLists[0]);

		// Presenting swap chain buffers in lockstep with frame buffers.
		surface.Present();

		u64& fenceValue = m_FenceValue;
		++fenceValue;

		CommandFrame& frame = m_CmdFrames[m_FrameIndex];
		frame.FenceValue = fenceValue;

		DXCall(m_CommandQueue->Signal(m_Fence, fenceValue));



		m_FrameIndex = (m_FrameIndex + 1) % c_FrameBufferCount;
	}

	void D3D12Command::CommandFrame::Wait(HANDLE fenceEvent, ID3D12Fence1* fence) const
	{
		ASSERT(fence && fenceEvent);
		// If the current fence value is still less than "FenceValue"
		// then we know that the GPU has not finished executing command lists
		// since it has not reached the "m_CommandQueue->Signal()" command.

		if (fence->GetCompletedValue() < FenceValue)
		{
			// We have the fence create an event which is signaled once the fence's current value equals "FenceValue"
			DXCall(fence->SetEventOnCompletion(FenceValue, fenceEvent))
				// Wait until the fence has triggered the event that its current value has reached "FenceValue"
				// indicating that the command queue has finished executing.
				WaitForSingleObject(fenceEvent, INFINITE);
		}
	}

	void D3D12Command::CommandFrame::Release()
	{
		Core::Release(CmdAllocator);
		FenceValue = 0;
	}
}
