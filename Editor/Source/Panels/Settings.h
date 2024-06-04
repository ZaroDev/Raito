#pragma once

#include "Panel.h"

namespace Editor
{
	class Settings final : public Panel
	{
	public:
		Settings() : Panel("Settings")
		{
		}

		virtual ~Settings() = default;

		void Update() override;
		void Render() override;

	private:
		bool m_SSAO = true;
		bool m_ParallaxMapping = true;
		bool m_Bloom = true;
		bool m_CascadeShadowMaps = true;
	};
}
