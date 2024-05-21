#include "pch.h"
#include "D3D12Command.h"
#include <Raito/Renderer/D3D12/D3D12Core.h>

#include "optick/include/optick.h"

Raito::Renderer::D3D12::D3D12Command::D3D12Command(D3D12Device* const device, D3D12_COMMAND_LIST_TYPE type)
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
		type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
		L"GFX Command queue" :
		type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
		L"Compute command queue" : L"Command queue");

	for (uint32_t i = 0; i < c_FrameBufferCount; i++)
	{
		CommandFrame& frame = m_CmdFrames[i];
		DXCall(hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&frame.CmdAllocator)));
		if (FAILED(hr))
		{
			Release();
		}
		NAME_D3D12_OBJECT_INDEXED(frame.CmdAllocator, i,
			type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
			L"GFX Command Allocator" :
			type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
			L"Compute command Allocator" : L"Command Allocator");
	}

	switch (type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT: m_CommandType = nvrhi::CommandQueue::Graphics; break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE: m_CommandType = nvrhi::CommandQueue::Compute; break;
	case D3D12_COMMAND_LIST_TYPE_COPY: m_CommandType = nvrhi::CommandQueue::Copy; break;
	}

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

Raito::Renderer::D3D12::D3D12Command::~D3D12Command()
{
	ASSERT(!m_CommandQueue && !m_CommandList && !m_Fence);
}

void Raito::Renderer::D3D12::D3D12Command::CreateCommandList()
{
	const auto params = nvrhi::CommandListParameters()
		.setQueueType(m_CommandType);
	
	m_CommandList = Core::NVDevice()->createCommandList(params);
}


void Raito::Renderer::D3D12::D3D12Command::Flush()
{
	for (uint32_t i = 0; i < c_FrameBufferCount; i++)
	{
		m_CmdFrames[i].Wait(m_FenceEvent, m_Fence);
	}
	m_FrameIndex = 0;
}

void Raito::Renderer::D3D12::D3D12Command::Release()
{
	Flush();
	Core::Release(m_Fence);
	m_FenceValue = 0;

	CloseHandle(m_FenceEvent);
	m_FenceEvent = nullptr;

	Core::Release(m_CommandQueue);

	for (uint32_t i = 0; i < c_FrameBufferCount; i++)
	{
		m_CmdFrames[i].Release();
	}
}

void Raito::Renderer::D3D12::D3D12Command::BeginFrame()
{
	OPTICK_EVENT();
	CommandFrame& frame = m_CmdFrames[m_FrameIndex];
	frame.Wait(m_FenceEvent, m_Fence);

	DXCall(frame.CmdAllocator->Reset());
	m_CommandList->open();
}

void Raito::Renderer::D3D12::D3D12Command::EndFrame(const D3D12Surface& surface)
{
	OPTICK_EVENT();
	m_CommandList->close();
	Core::NVDevice()->executeCommandList(m_CommandList, m_CommandType);
	// Presenting swap chain buffers in lockstep with frame buffers.
	surface.Present();

	u64& fenceValue = m_FenceValue;
	++fenceValue;

	CommandFrame& frame = m_CmdFrames[m_FrameIndex];
	frame.FenceValue = fenceValue;
	m_CommandQueue->Signal(m_Fence, fenceValue);

	m_FrameIndex = (m_FrameIndex + 1) % c_FrameBufferCount;
}

void Raito::Renderer::D3D12::D3D12Command::CommandFrame::Wait(HANDLE fenceEvent, ID3D12Fence1* fence) const
{
	OPTICK_EVENT();
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

void Raito::Renderer::D3D12::D3D12Command::CommandFrame::Release()
{
	Core::Release(CmdAllocator);
	FenceValue = 0;
}
