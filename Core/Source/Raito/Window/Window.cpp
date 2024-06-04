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

#include "glad/glad.h"

#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include "optick/include/optick.h"

#include "Renderer/Renderer.h"


namespace Raito
{
	namespace
	{
		bool g_Initialized = false;

		WindowInfo g_DefaultInfo{};
		u32 g_MainWindow{};

		std::vector<Renderer::RenderSurface> g_RenderSurfaces{};

		void GlfwErrorCallback(int error, const char* description)
		{
			ERR("GLFW", "{}, {}", error, description);
		}

		void GlfwWindowSizeCallback(GLFWwindow* win, int width, int height)
		{
			SysWindow* data = static_cast<SysWindow*>(glfwGetWindowUserPointer(win));
			data->Info.Width = width;
			data->Info.Height = height;
			data->Surface->Surface.Resize(width, height);
		}

		void DestroySurface(Renderer::RenderSurface* surface)
		{
			const auto win = static_cast<GLFWwindow*>(surface->Window->Window);
			glfwDestroyWindow(win);
			Renderer::RemoveSurface(surface->Surface.Id());


			g_RenderSurfaces.erase(g_RenderSurfaces.begin() + surface->Window->Id);

			delete surface->Window;
			surface->Window = nullptr;
		}
	}

	namespace Window
	{
		bool Initialize(Renderer::API api, const WindowInfo& defaultInfo)
		{
			if (g_Initialized)
			{
				F_ERR("Window module was already initialized");
				return false;
			}

			g_DefaultInfo = defaultInfo;
			// Initialize GLFW

			if (glfwInit() != GLFW_TRUE)
			{
				F_ERR("Error while initializing GLFW with code: {}", glfwGetError(NULL));
				return false;
			}
			glfwSetErrorCallback(GlfwErrorCallback);

			if (api != Renderer::API::OPENGL)
			{
				glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			}
			else
			{
#ifndef DIST
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
			}

			

			// Create the GLFW g_Window
			GLFWwindow* win = nullptr;
			if ((win = glfwCreateWindow((int)defaultInfo.Width, (int)defaultInfo.Height, defaultInfo.Title.c_str(), nullptr, nullptr)) == nullptr)
			{
				F_ERR("Error while creating a g_Window {}", glfwGetError(NULL));
				return false;
			}

			if (api == Renderer::API::OPENGL)
			{
				glfwMakeContextCurrent(win);
				if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
				{
					ERR("Window", "Failed to initialize glad");
					return false;
				}
				glfwSwapInterval(false);
			}

			// Create and register the window
			auto* window = new SysWindow();
			window->Info = defaultInfo;
			window->Window = win;
			window->WindowHandle = glfwGetWin32Window(win);
			window->Id = static_cast<u32>(g_RenderSurfaces.size());

			Renderer::RenderSurface surface{ window, Renderer::CreateSurface(window) };

			window->Surface = &g_RenderSurfaces.emplace_back(surface);

			glfwSetWindowUserPointer(win, window);
			glfwSetWindowSizeCallback(win, GlfwWindowSizeCallback);

			g_MainWindow = window->Id;

			g_Initialized = true;

			Renderer::Initialize();

			return true;
		}

		bool Update()
		{
			OPTICK_EVENT();

			for (auto& renderSurface : g_RenderSurfaces)
			{
				if (Renderer::GetCurrentAPI() == Renderer::API::OPENGL)
				{
					glfwSwapBuffers(static_cast<GLFWwindow*>(renderSurface.Window->Window));
				}
				renderSurface.Surface.Render();

				if (glfwWindowShouldClose(static_cast<GLFWwindow*>(renderSurface.Window->Window)))
				{
					if (renderSurface.Window->Id == g_MainWindow)
					{
						Core::Application::Get().Close();
						return false;
					}
					DestroySurface(&renderSurface);
				}

			}

			glfwPollEvents();
			return true;
		}

		void Shutdown()
		{
			for (auto& renderSurface : g_RenderSurfaces)
			{
				DestroySurface(&renderSurface);
			}

			g_RenderSurfaces.clear();
			g_Initialized = false;

			glfwTerminate();
		}

		u32 CreateNewWindow(const WindowInfo& info)
		{
			// Create the GLFW g_Window
			GLFWwindow* win = nullptr;
			if ((win = glfwCreateWindow((int)info.Width, (int)info.Height, info.Title.c_str(), nullptr, nullptr)) == nullptr)
			{
				F_ERR("Error while creating a g_Window {}", glfwGetError(NULL));
				return U32_MAX;
			}

			// Create and register the window
			SysWindow* window = new SysWindow();
			window->Info = info;
			window->Window = win;
			window->WindowHandle = glfwGetWin32Window(win);
			window->Id = static_cast<u32>(g_RenderSurfaces.size());

			Renderer::RenderSurface surface{ window, Renderer::CreateSurface(window) };

			window->Surface = &g_RenderSurfaces.emplace_back(surface);

			glfwSetWindowUserPointer(win, window);
			glfwSetWindowSizeCallback(win, GlfwWindowSizeCallback);

			return window->Id;
		}

		SysWindow& GetWindow(const u32 id)
		{
			if (id == 0)
			{
				return *g_RenderSurfaces[g_MainWindow].Window;
			}

			return *g_RenderSurfaces[id].Window;
		}

		void SetFullScreen(bool value)
		{
			
		}

		void SetVSync(bool value)
		{
			glfwSwapInterval(value);
		}
	}
}
