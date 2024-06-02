#include "Framebuffers.h"
#include <imgui.h>

#include <Raito/Raito.h>

namespace Editor
{
	void Framebuffers::Update()
	{
	}

	void Framebuffers::Render()
	{
		Raito::SysWindow window = Raito::Window::GetWindow();

		ImGui::Begin(m_Name.c_str(), &m_Open);

		static i32 zoom = 5u;

		ImGui::SliderInt("Zoom", &zoom, 1, 10);

		const float width = (float)(window.Info.Width / zoom);
		const float height = (float)(window.Info.Height / zoom);

		const ImVec2 size = { width, height };
		constexpr ImVec2 uv0 = { 0, 1 };
		constexpr ImVec2 uv1 = { 1, 0 };


		ImGui::Text("Position buffer");
		ImGui::Image((ImTextureID)(intptr_t)window.Surface->Surface.DeferredAttachment(), size, uv0, uv1);

		ImGui::Text("Normal buffer");
		ImGui::Image((ImTextureID)(intptr_t)window.Surface->Surface.DeferredAttachment(1), size, uv0, uv1);

		ImGui::Text("Albedo buffer");
		ImGui::Image((ImTextureID)(intptr_t)window.Surface->Surface.DeferredAttachment(2), size, uv0, uv1);

		ImGui::Text("Emissive buffer");
		ImGui::Image((ImTextureID)(intptr_t)window.Surface->Surface.DeferredAttachment(3), size, uv0, uv1);

		ImGui::Text("RougMetalAO buffer");
		ImGui::Image((ImTextureID)(intptr_t)window.Surface->Surface.DeferredAttachment(4), size, uv0, uv1);


		ImGui::Text("SSAO buffer");
		ImGui::Image((ImTextureID)(intptr_t)window.Surface->Surface.SSAOAttachment(), size, uv0, uv1);


		ImGui::Text("Depth buffer");
		ImGui::Image((ImTextureID)(intptr_t)window.Surface->Surface.DeferredDepth(), size, uv0, uv1);

		

		ImGui::End();
	}
}
