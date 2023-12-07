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
#include "Window.h"
#include "Core/Application.h"

#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"



namespace Raito
{
	namespace
	{
		std::vector<SysWindow> Windows{};
		WindowInfo DefaultInfo{};
		u32 MainWindow{};

		bool Initialized = false;

		void GLFWErrorCallback(int error, const char* description)
		{
			ERR("GLFW", "{}, {}", error, description);
		}

		void DestroyWindow(SysWindow* window)
		{
			GLFWwindow* win = (GLFWwindow*)window->WindowHandle;
			glfwDestroyWindow(win);
			Windows.erase(Windows.begin() + window->ID);

		}
	}

	namespace Window
	{
		bool Initialize(WindowInfo defaultInfo)
		{
			if (Initialized)
			{
				F_ERR("Window module was already initialized");
				return false;
			}

			DefaultInfo = defaultInfo;
			// Initialize GLFW
			if (glfwInit() != GLFW_TRUE)
			{
				F_ERR("Error while initializing GLFW with code: {}", glfwGetError(NULL));
				return false;
			}
			glfwWindowHint(GLFW_NO_API, GLFW_TRUE);

			MainWindow = CreateNewWindow(defaultInfo);
			
			Initialized = true;

			return true;
		}

		bool Update()
		{

			for (u32 i = 0; i < Windows.size(); i++)
			{
				if (Windows[i].IsAlive)
				{
					glfwPollEvents();
					continue;
				}
				DestroyWindow(&Windows[i]);
			}

			return true;
		}

		void Shutdown()
		{
			for (u32 i = 0; i < Windows.size(); i++)
			{
				DestroyWindow(&Windows[i]);
			}
		}

		u32 CreateNewWindow(WindowInfo info)
		{
			GLFWwindow* win = nullptr;
			if ((win = glfwCreateWindow((int)info.Width, (int)info.Height, info.Title.c_str(), nullptr, nullptr)) == nullptr)
			{
				F_ERR("Error while creating a window {}", glfwGetError(NULL));
				return U32_MAX;
			}

			
			SysWindow window{};
			window.Info = info;
			window.Name = info.Title;
			window.Window = win;
			window.WindowHandle = glfwGetWin32Window(win);
			window.ID = Windows.size();
			window.IsAlive = true;
			Windows.emplace_back(window);

			glfwSetWindowUserPointer(win, &Windows[window.ID]);
			glfwSetWindowCloseCallback(win, [](GLFWwindow* window)
			{
				SysWindow& win = *(SysWindow*)glfwGetWindowUserPointer(window);
				if (win.ID == MainWindow)
				{
					Core::Application::Get().Close();
				}
				win.IsAlive = false;
			});


			return window.ID;
		}
	}
}