#include "Settings.h"
#include <imgui.h>

#include "Raito/Renderer/Renderer.h"

namespace Editor
{
	void Settings::Update()
	{
	}

	void Settings::Render()
	{
		ImGui::Begin(m_Name.c_str(), &m_Open);

		ImGui::Text("Render effects");
		ImGui::Checkbox("Parallax mapping", &m_ParallaxMapping);
		ImGui::Separator();

		ImGui::Text("General");
		if(ImGui::Checkbox("SSAO", &m_SSAO))
		{
			Raito::Renderer::SetSSAO(m_SSAO);
		}

		if(ImGui::Checkbox("Cascade shadow maps", &m_CascadeShadowMaps))
		{
			Raito::Renderer::SetShadows(m_CascadeShadowMaps);
		}

		if(ImGui::Checkbox("Frustum culling", &m_FrustumCulling))
		{
			Raito::Renderer::EnableCulling(m_FrustumCulling);
		}

		ImGui::Text("Post processing");

		if(ImGui::Checkbox("Bloom", &m_Bloom))
		{
			Raito::Renderer::SetBloom(m_Bloom);
		}


		ImGui::Separator();
		ImGui::Text("Debug");

		if(ImGui::Checkbox("Show AABB", &m_DebugAABB))
		{
			Raito::Renderer::EnableDebugAABB(m_DebugAABB);
		}

		ImGui::End();
	}
}
