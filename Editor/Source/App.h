#pragma once
#include <Raito/Raito.h>

namespace Editor
{
	class App : public Raito::Core::Application
	{
	public:
		App();

	protected:
		bool OnInit() override;
		bool OnUpdate() override;
		bool OnRenderGUI() override;
		void OnShutdown() override;

	private:
		// Test scene
		Raito::ECS::Scene m_Scene{};
	};
}