#pragma once

namespace Raito::Renderer
{
	//! Renderer Api enumerator
	enum class API : u8
	{
		NONE = 0,
		D3D12,
		OPENGL,
		COUNT
	};
}