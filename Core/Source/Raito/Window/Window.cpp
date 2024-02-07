#include "pch.h"
#include "Window.h"
#include "Core/Application.h"

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

		std::vector<SysWindow> g_Windows{};

		void GlfwErrorCallback(int error, const char* description)
		{
			ERR("GLFW", "{}, {}", error, description);
		}

		void DestroyWindow(const SysWindow* window)
		{
			const auto win = static_cast<GLFWwindow*>(window->Window);
			glfwDestroyWindow(win);
			g_Windows.erase(g_Windows.begin() + window->Id);
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
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
			}


			g_MainWindow = CreateNewWindow(defaultInfo);
			
			g_Initialized = true;

			return true;
		}

		bool Update()
		{
			for (const auto& window : g_Windows)
			{
				if(Renderer::GetCurrentAPI() == Renderer::API::OPENGL)
				{
					glfwSwapBuffers(static_cast<GLFWwindow*>(window.Window));
				}

				if(glfwWindowShouldClose(static_cast<GLFWwindow*>(window.Window)))
				{
					if(window.Id == g_MainWindow)
					{
						Core::Application::Get().Close();
						return false;
					}
					DestroyWindow(&window);
				}
			}

			glfwPollEvents();
			return true;
		}

		void Shutdown()
		{
			for (auto& window : g_Windows)
			{
				DestroyWindow(&window);
			}

			g_Windows.clear();
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
			SysWindow window{};
			window.Info = info;
			window.Window = win;
			window.WindowHandle = glfwGetWin32Window(win);
			window.Id = static_cast<u32>(g_Windows.size());

			g_Windows.emplace_back(window);


			return window.Id;
		}

		SysWindow& GetWindow(const u32 id)
		{
			return g_Windows[id];
		}
	}
}
