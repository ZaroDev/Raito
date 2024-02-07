#include <pch.h>
#include "OpenGLCore.h"

#include <glad/glad.h>

#include "GLFW/glfw3.h"

namespace Raito::Renderer::OpenGL
{
	bool Initialize()
	{
		if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			O_ERROR("Failed to initialize glad");
			return false;
		}

		return true;
	}

	void Shutdown()
	{
		
	}

}
