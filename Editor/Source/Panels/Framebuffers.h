#pragma once

#include "Panel.h"

namespace Editor
{
	class Framebuffers final : public Panel
	{
	public:
		Framebuffers() : Panel("Framebuffers")
		{
		}

		virtual ~Framebuffers() = default;

		void Update() override;
		void Render() override;


	};
}