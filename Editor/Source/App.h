#pragma once
#include <Raito/Raito.h>

namespace Editor
{
	class App : public Raito::Core::Application
	{
	public:
		App();

	protected:
		bool OnUpdate() override;
		bool OnRenderGUI() override;

	};
}