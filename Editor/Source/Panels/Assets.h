#pragma once

#include "Panel.h"

namespace Editor
{
	class Assets final : public Panel
	{
	public:
		Assets() : Panel("Assets")
		{
		}

		virtual ~Assets() = default;

		void Update() override;
		void Render() override;
	};
}
