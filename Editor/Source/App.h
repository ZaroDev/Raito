#pragma once
#include <Raito/Raito.h>

namespace Editor
{
	class App final : public Raito::Core::Application
	{
	public:
		App() = default;

	protected:
		bool OnInit() override;
		bool OnUpdate() override;
		bool OnRenderGUI() override;
		void OnShutdown() override;

	private:
		// Test scene
		Raito::ECS::Scene m_Scene{};

		std::vector<std::unique_ptr<class Panel>> m_Panels{};
	};
}