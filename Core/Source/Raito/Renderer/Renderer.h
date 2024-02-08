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

#include "GraphicsAPI.h"

namespace Raito
{
	
	struct SysWindow;

	namespace Renderer
	{
		class Surface
		{
		public:
			constexpr explicit Surface(u32 id) : m_Id(id) {}
			Surface() = default;

			NODISCARD constexpr u32 Id() const { return m_Id; }

			void Resize(u32 width, u32 height) const;
			NODISCARD u32 Width() const;
			NODISCARD u32 Height() const;
			void Render() const;
		private:
			u32 m_Id;
		};

		struct RenderSurface
		{
			SysWindow* Window;
			Surface Surface;
		};
		//
		//! @param api The desired api to be used 
		bool SetPlatformInterface(API api);

		//! Renderer initialization function
		//! @returns the success
		bool Initialize();

		//! Renderer shutdown function
		//! Releases all renderer objects
		void Shutdown();

		//! Renderer graphics API getter
		//! @return Current graphics API
		NODISCARD API GetCurrentAPI();

		Surface CreateSurface(SysWindow* window);
		void RemoveSurface(u32 id);
	}
}
