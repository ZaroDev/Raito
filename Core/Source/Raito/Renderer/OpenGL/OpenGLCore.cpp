#include <pch.h>
#include "OpenGLCore.h"

#include <glad/glad.h>

#include "OpenGLShaderCompiler.h"
#include "GLFW/glfw3.h"

#include "OpenGLObjects/OpenGLFrameBuffer.h"
#include "OpenGLObjects/OpenGLShader.h"
#include "Window/Window.h"

namespace Raito::Renderer::OpenGL
{
	namespace
	{
		std::vector<OpenGLFrameBuffer> g_Surfaces{};

		void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
		{
			switch (severity)
			{
			case GL_DEBUG_SEVERITY_HIGH:         O_ERROR("{}", message); return;
			case GL_DEBUG_SEVERITY_MEDIUM:       O_WARN("{}", message); return;
			case GL_DEBUG_SEVERITY_LOW:          O_WARN("{}", message); return;
			case GL_DEBUG_SEVERITY_NOTIFICATION: O_LOG("{}", message); return;
			}

			ASSERT(false);
		}

		u32 g_FrameBufferQuadVAO, g_FrameBufferQuadVBO;

		void InitializePostProcessPass()
		{
			constexpr float quadVertices[] = {
				// positions   // texCoords
				-1.0f,  1.0f,  0.0f, 1.0f,
				-1.0f, -1.0f,  0.0f, 0.0f,
				1.0f, -1.0f,  1.0f, 0.0f,

				-1.0f,  1.0f,  0.0f, 1.0f,
				1.0f, -1.0f,  1.0f, 0.0f,
				1.0f,  1.0f,  1.0f, 1.0f
			};

			glGenVertexArrays(1, &g_FrameBufferQuadVAO);
			glGenBuffers(1, &g_FrameBufferQuadVBO);
			glBindVertexArray(g_FrameBufferQuadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, g_FrameBufferQuadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		}

		void ShutdownPostProcessPass()
		{
			glDeleteVertexArrays(1, &g_FrameBufferQuadVAO);
			glDeleteBuffers(1, &g_FrameBufferQuadVBO);
		}
	}

	bool Initialize()
	{
		O_LOG("OpenGL info");
		O_LOG("Vendor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		O_LOG("Renderer {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		O_LOG("Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

#ifndef DIST
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

		// Enable blend and depth testing
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);


		if (!ShaderCompiler::Initialize())
		{
			O_ERROR("Couldn't initalize shader compilation");
			return false;
		}

		InitializePostProcessPass();

		return true;
	}

	void Shutdown()
	{
		ShaderCompiler::Shutdown();
	}

	Surface CreateSurface(SysWindow* window)
	{
		FrameBufferData data;
		data.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RED_INTEGER, FrameBufferTextureFormat::Depth };
		data.Width = window->Info.Width;
		data.Height = window->Info.Height;

		g_Surfaces.emplace_back(data);

		return Surface{ (u32)g_Surfaces.size() - 1 };
	}

	void RemoveSurface(u32 id)
	{
		g_Surfaces.erase(g_Surfaces.begin() + id);
	}

	void ResizeSurface(u32 id, u32 width, u32 height)
	{
		g_Surfaces[id].Resize(width, height);
	}

	u32 SurfaceWidth(u32 id)
	{
		return g_Surfaces[id].Data().Width;
	}

	u32 SurfaceHeight(u32 id)
	{
		return g_Surfaces[id].Data().Height;
	}

	void RenderSurface(u32 id)
	{
		const OpenGLFrameBuffer& buffer = g_Surfaces[id];
		buffer.Bind();

		glClearColor(1.0f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


		buffer.UnBind();


		// Post-processing pass
		{
			// Clear the back buffer
			glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
			glClear(GL_COLOR_BUFFER_BIT);


			const auto shader = static_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::POST_PROCESS));

			shader->Bind();

			glBindVertexArray(g_FrameBufferQuadVAO);
			glDisable(GL_DEPTH_TEST);
			glBindTexture(GL_TEXTURE_2D, buffer.ColorAttachment());
			glDrawArrays(GL_TRIANGLES, 0, 6);

			shader->UnBind();
		}

	}
}
