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
		bool m_HideGUI = false;

		std::vector<std::unique_ptr<class Panel>> m_Panels{};
	};
}