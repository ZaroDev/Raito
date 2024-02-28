#pragma once
#include <string>

#include "Raito/Core/Common.h"

namespace Editor
{
	class Panel
	{
	public:
		explicit Panel(const char* name) : m_Name(name){}
		virtual ~Panel() = default;

		DEFAULT_MOVE_AND_COPY(Panel)

		virtual	void Update(){}
		virtual void Render(){}

	protected:
		std::string m_Name;
		bool m_Open = true;
	};
}
