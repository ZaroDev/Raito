#include "Settings.h"
#include <imgui.h>

#include "Raito/Renderer/Renderer.h"

namespace Editor
{
	Settings::Settings() : Panel("Settings")
	{
		m_Technique = Raito::Renderer::GetCurrentRenderType();
	}

	void Settings::Update()
	{
	}

	void Settings::Render()
	{
		ImGui::Begin(m_Name.c_str(), &m_Open);

		const char* types[] =
		{
			"Forward",
			"Deferred",
			"Deferred Plus"
		};
		static const char* currentItem = types[static_cast<int>(m_Technique)];
		if (ImGui::BeginCombo("Render type", currentItem))
		{
			for (int i = 0; i < IM_ARRAYSIZE(types); i++)
			{
				const bool isSelected = (currentItem == types[i]);
				if (ImGui::Selectable(types[i], isSelected))
				{
					currentItem = types[i];
					m_Technique = static_cast<Raito::Renderer::LightTechnique>(i);
					Raito::Renderer::SetRenderType(m_Technique);
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		ImGui::Text("Render effects");
		if (ImGui::Checkbox("Parallax mapping", &m_ParallaxMapping)) {
			Raito::Renderer::SetParallaxMapping(m_ParallaxMapping);
		}
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
