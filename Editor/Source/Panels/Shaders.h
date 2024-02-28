#pragma once

#include "Panel.h"

namespace Editor
{
	class Shaders final : public Panel
	{
	public:
		Shaders() : Panel("Shaders")
		{
		}

		virtual ~Shaders() = default;

		void Update() override;
		void Render() override;
	};
}
