#include "Shaders.h"
#include <imgui.h>
#include <vector>

#include <Raito/Raito.h>

namespace Editor
{
	void Shaders::Update()
	{
	}

	void Shaders::Render()
	{
		ImGui::Begin(m_Name.c_str(), &m_Open);

		const auto& shaders = Raito::Renderer::GetAllShaders();

		ImGui::BeginTable("Compiled Shaders", 1);
		ImGui::TableNextColumn();
		for(const auto& shader : shaders)
		{
			ImGui::PushID(shader->EngineType());
			ImGui::Text(shader->FilePath().c_str());
			ImGui::SameLine();
			if(ImGui::Button("Refresh"))
			{
				CompileShader(GetFileData(shader->EngineType()));
			}
			ImGui::PopID();
		}

		ImGui::EndTable();

		ImGui::End();
	}
}
