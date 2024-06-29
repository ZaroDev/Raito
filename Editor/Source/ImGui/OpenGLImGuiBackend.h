#pragma once


namespace Editor::ImGuiBackend
{
	struct ImGuiBackendRHI;
	namespace OpenGL
	{
		void GetOpenGLBackend(ImGuiBackendRHI& interface);
	
		void InitOpenGL();
		void BeginOpenGL();
		void RenderOpenGL();
		void ShutdownOpenGL();
	}
}
