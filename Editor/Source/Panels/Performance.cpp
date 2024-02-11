#include "Performance.h"
#include <imgui.h>

#include <Raito/Time/Time.h>

namespace Editor
{
	Performance::~Performance()
	{
		m_FPSLog.clear();
		m_DeltaTimeLog.clear();
	}

	void Performance::Update()
	{
		if(m_FPSLog.size() >= 30)
		{
			m_FPSLog.erase(m_FPSLog.begin());
			m_DeltaTimeLog.erase(m_DeltaTimeLog.begin());
		}


		m_DeltaTimeLog.emplace_back(Raito::Time::GetDeltaTime());
		m_FPSLog.emplace_back(Raito::Time::GetFPS());
	}

	void Performance::Render()
	{
		ImGui::Begin(m_Name.c_str(), &m_Open);

		ImGui::Text("Delta Time: %.2f ms", Raito::Time::GetDeltaTime());
		ImGui::Text("FPS: %.2f", Raito::Time::GetFPS());

		ImGui::PlotLines("##dt_log", m_DeltaTimeLog.data(), m_DeltaTimeLog.size());
		ImGui::PlotLines("##fps_log", m_FPSLog.data(), m_FPSLog.size());

		ImGui::End();
	}
}
