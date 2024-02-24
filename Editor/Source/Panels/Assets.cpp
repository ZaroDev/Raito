#include "Assets.h"
#include <imgui.h>

#include <filesystem>
#include <unordered_map>
#include "Raito/Assets/AssetImport.h"

namespace Editor
{
	void Assets::Update()
	{
	}

	void Assets::Render()
	{
		ImGui::Begin(m_Name.c_str(), &m_Open);

		ImGui::Text("Textures");
		const auto& textures = Raito::Assets::GetAllTextures();
		for (const auto& texture : textures)
		{
			if (texture.second)
			{
				ImGui::Image((ImTextureID)(intptr_t)texture.second->RenderId,
					ImVec2{ static_cast<float>(texture.second->Width / 10u), static_cast<float>(texture.second->Height / 10u) });
				ImGui::Text("%i", texture.second->RenderId);
			}
			ImGui::Text("Path %s", texture.first.string().c_str());
		}


		ImGui::End();
	}
}
