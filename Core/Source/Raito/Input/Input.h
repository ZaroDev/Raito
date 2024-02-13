#pragma once

#include "KeyCodes.h"

#include <Math/MathTypes.h>

namespace Raito::Input
{
	bool IsKeyDown(KeyCode keycode);
	bool IsMouseButtonDown(MouseButton button);

	V2 GetMousePosition();

	void SetCursorMode(CursorMode mode);
};