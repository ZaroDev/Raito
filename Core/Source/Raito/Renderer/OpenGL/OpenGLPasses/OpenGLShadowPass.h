#pragma once

#include <Raito/Renderer/OpenGL/OpenGLCore.h>

namespace Raito::Renderer
{
	class Camera;
}

namespace Raito::Renderer::OpenGL::Shadows
{
	struct CascadeUniforms
	{
		GLint FarPlane;
		GLint CascadeCount;
		GLint View;
		GLint LightDir;
	};


	bool Initialize();
	void Enable(bool value);
	void Update(Camera* camera);
	void Shutdown();
	u32 GetLightSpaceMatricesUBO();
	u32 GetShadowMap();
	const std::vector<float>& GetCascadeLevels();
}