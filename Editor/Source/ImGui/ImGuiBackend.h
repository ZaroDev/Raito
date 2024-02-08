#pragma once

namespace Editor::ImGuiBackend
{
	struct ImGuiBackendRHI
	{
		void(*Init)();
		void(*NewFrame)();
		void(*Render)();
		void(*Shutdown)();
	};


	void Init();
	void NewFrame();
	void Render();
	void Shutdown();
}
