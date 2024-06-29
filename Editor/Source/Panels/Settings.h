#pragma once

#include "Panel.h"
#include <Raito/Raito.h>

namespace Editor
{
	class Settings final : public Panel
	{
	public:
		Settings();

		virtual ~Settings() = default;

		void Update() override;
		void Render() override;

	private:
		bool m_SSAO = true;
		bool m_ParallaxMapping = true;
		bool m_Bloom = true;
		bool m_CascadeShadowMaps = true;
		bool m_FrustumCulling = true;
		bool m_DebugAABB = false;
		Raito::Renderer::LightTechnique m_Technique = Raito::Renderer::LightTechnique::DeferredPlus;
	};
}
