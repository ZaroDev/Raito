#include "pch.h"
#include "Window.h"
#include "Core/Application.h"

#include "glad/glad.h"

#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

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

		void DestroySurface(const Renderer::RenderSurface* surface)
		{
			const auto win = static_cast<GLFWwindow*>(surface->Window->Window);
			glfwDestroyWindow(win);
			Renderer::RemoveSurface(surface->Surface.Id());
			g_RenderSurfaces.erase(g_RenderSurfaces.begin() + surface->Window->Id);
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

			if(api != Renderer::API::OPENGL)
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
				return U32_MAX;
			}
			glfwMakeContextCurrent(win);

			if (api == Renderer::API::OPENGL)
			{
				if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
				{
					ERR("Window", "Failed to initialize glad");
					return false;
				}
			}

			// Create and register the window
			SysWindow* window = new SysWindow();
			window->Info = defaultInfo;
			window->Window = win;
			window->WindowHandle = glfwGetWin32Window(win);
			window->Id = static_cast<u32>(g_RenderSurfaces.size());

			Renderer::RenderSurface surface{ window, Renderer::CreateSurface(window) };

			g_RenderSurfaces.emplace_back(surface);

			g_MainWindow = window->Id;

			g_Initialized = true;

			return true;
		}

		bool Update()
		{
			for (const auto& renderSurface : g_RenderSurfaces)
			{
				if(Renderer::GetCurrentAPI() == Renderer::API::OPENGL)
				{
					glfwSwapBuffers(static_cast<GLFWwindow*>(renderSurface.Window->Window));
				}

				if(glfwWindowShouldClose(static_cast<GLFWwindow*>(renderSurface.Window->Window)))
				{
					if(renderSurface.Window->Id == g_MainWindow)
					{
						Core::Application::Get().Close();
						return false;
					}
					DestroySurface(&renderSurface);
				}

				renderSurface.Surface.Render();
			}

			glfwPollEvents();
			return true;
		}

		void Shutdown()
		{
			for (const auto& renderSurface : g_RenderSurfaces)
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
			glfwMakeContextCurrent(win);

			// Create and register the window
			SysWindow* window = new SysWindow();
			window->Info = info;
			window->Window = win;
			window->WindowHandle = glfwGetWin32Window(win);
			window->Id = static_cast<u32>(g_RenderSurfaces.size());

			Renderer::RenderSurface surface{ window, Renderer::CreateSurface(window) };

			g_RenderSurfaces.emplace_back(surface);


			return window->Id;
		}

		SysWindow& GetWindow(const u32 id)
		{
			return *g_RenderSurfaces[id].Window;
		}
	}
}
