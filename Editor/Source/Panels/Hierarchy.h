#pragma once

#include "Panel.h"

namespace Editor
{
	class Hierarchy final : public Panel
	{
	public:
		Hierarchy() : Panel("Hierarchy")
		{
		}

		virtual ~Hierarchy() = default;

		void Update() override;
		void Render() override;
	};
}
