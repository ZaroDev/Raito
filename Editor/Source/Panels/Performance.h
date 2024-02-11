#pragma once

#include <vector>

#include "Panel.h"

namespace Editor
{
	class Performance final : public Panel
	{
	public:
		Performance() : Panel("Performance"){}
		~Performance();

		DEFAULT_MOVE_AND_COPY(Performance)

		void Update() override;
		void Render() override;

	private:
		std::vector<float> m_DeltaTimeLog{};
		std::vector<float> m_FPSLog{};
	};
}
