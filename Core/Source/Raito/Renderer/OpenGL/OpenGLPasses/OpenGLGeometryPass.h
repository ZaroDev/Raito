#pragma once

#include <Raito/Renderer/OpenGL/OpenGLCommon.h>

#include "ECS/Scene.h"

namespace Raito::Renderer::OpenGL::Geometry
{
	void EnableAABBDebug(bool value);
	void EnableCulling(bool value);
	void Update(const ECS::Scene& scene, const Camera& camera, const OpenGLFrameBuffer& buffer);
}
