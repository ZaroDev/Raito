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

#pragma once

#include <Raito/Renderer/D3D12/D3D12Common.h>

namespace Raito::Renderer::D3D12
{
	class D3D12Command
	{
	public:
		D3D12Command() = default;

		DISABLE_MOVE_AND_COPY(D3D12Command);

		explicit D3D12Command(D3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
		~D3D12Command();

		void Flush();
		void Release();

		void BeginFrame();
		void EndFrame();

		NODISCARD constexpr ID3D12CommandQueue* CommandQueue() const { return m_CommandQueue; }


	private:

		struct CommandFrame
		{
			ID3D12CommandAllocator* CmdAllocator = nullptr;
			u64 FenceValue = 0;

			void Wait(HANDLE fenceEvent, ID3D12Fence1* fence) const;
			void Release();
		};

		ID3D12CommandQueue* m_CommandQueue = nullptr;
		D3D12GraphicsCommandList* m_CommandList = nullptr;

		ID3D12Fence1* m_Fence = nullptr;
		u64 m_FenceValue = 0;
		HANDLE m_FenceEvent = {};

		CommandFrame m_CmdFrames[c_FrameBufferCount] = {};
		u32 m_FrameIndex = 0;
	};
}