#include "Assets.h"
#include <imgui.h>

namespace Editor
{
	void Assets::Update()
	{
	}

	void Assets::Render()
	{
		ImGui::Begin(m_Name.c_str(), &m_Open);




		ImGui::End();
	}
}
