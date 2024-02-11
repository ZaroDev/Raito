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

#include "Renderer/GraphicsAPI.h"

namespace Raito
{
	//! Window info structure
	struct WindowInfo
	{
		std::string Title{}; /**< Title of the window*/

		u32 Height{}; /**< Height of the window in pixels */
		u32 Width{}; /**< Width of the window in pixels */
		
		bool Fullscreen = false; /**< Flag for fullscreen mode */
		bool VSync = false; /**< Flag for VSync mode */
	};

	//! System window structure
	struct SysWindow
	{
		WindowInfo Info; /**< Information about the window */

		void* WindowHandle = nullptr; /**< OS window handle */
		void* Window = nullptr; /**< Internal window pointer */

		u32 Id{}; /**< ID of the window */
	};

	

	namespace Window
	{
		//! Initialization function for the window module
		//! @param api Graphics API of the application
		//! @param defaultInfo Window information for the main window
		//! @return Boolean indicating success value
		bool Initialize(Renderer::API api, const WindowInfo& defaultInfo = {});

		void CreateMainWindow();

		//! EndTimeUpdate function for the window module
		//! Polls the events for all the windows
		//! @return Boolean indicating success value
		bool Update();

		//! Shutdown function for the window module
		//! Deletes all the created windows
		void Shutdown();

		//! Creates a window and returns the id
		//! @param info The information for the window to be created
		//! @return ID of the created window
		u32 CreateNewWindow(const WindowInfo& info);

		//! Returns the window with a given ID
		//! @param id ID of the window to get returned
		//! @return Window of the given ID
		SysWindow& GetWindow(const u32 id);
	}
}
