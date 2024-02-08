#include "OpenGLImGuiBackend.h"
#include "ImGuiBackend.h"
#include <Raito/Raito.h>

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "Raito/Window/Window.h"

namespace Editor::ImGuiBackend::OpenGL
{
	void GetOpenGLBackend(ImGuiBackendRHI& interface)
	{
		interface.Init = InitOpenGL;
		interface.Shutdown = ShutdownOpenGL;
		interface.NewFrame = BeginOpenGL;
		interface.Render = RenderOpenGL;
	}

	void InitOpenGL()
	{
		ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(Raito::Window::GetWindow(0).Window), true);
		ImGui_ImplOpenGL3_Init("#version 460");
	}

	void BeginOpenGL()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void RenderOpenGL()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void ShutdownOpenGL()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}
