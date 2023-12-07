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

namespace Raito
{
	struct WindowInfo
	{
		std::string Title{};
		u32 Height{};
		u32 Width{};
		bool Fullscreen = false;
		bool VSync = false;
	};

	struct SysWindow
	{
		void* WindowHandle = nullptr;
		void* Window = nullptr;
		
		WindowInfo Info;

		std::string Name{};

		u32 ID{};

		bool IsAlive = false;
	};

	

	namespace Window
	{
		bool Initialize(WindowInfo defaultInfo = {});
		bool Update();
		void Shutdown();

		/// <summary>
		/// Creates a window and returns the id
		/// </summary>
		u32 CreateNewWindow(WindowInfo info);

		/// <summary>
		/// Returns the window with a given ID
		/// </summary>
		SysWindow& GetWindow(u32 id);
	}
	

}
