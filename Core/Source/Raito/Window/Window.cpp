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
			CreateNewWindow({.Title = "1", .Height = 500, .Width = 500});
			CreateNewWindow({ .Title = "2", .Height = 500, .Width = 500 });
			CreateNewWindow({ .Title = "3", .Height = 500, .Width = 500 });
			CreateNewWindow({ .Title = "4", .Height = 500, .Width = 500 });


			Initialized = true;

			return true;
		}

		bool Update()
		{
			for (u32 i = 0; i < Windows.size(); i++)
			{
				if (!Windows[i].IsAlive)
				{
					DestroyWindow(&Windows[i]);
				}
			}

			glfwPollEvents();
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