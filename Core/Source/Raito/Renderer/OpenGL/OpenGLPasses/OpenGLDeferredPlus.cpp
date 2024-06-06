#include <pch.h>
#include "OpenGLDeferredPlus.h"

#include "OpenGLGeometryPass.h"
#include "Core/Application.h"
#include "Renderer/OpenGL/OpenGLObjects/OpenGLShader.h"

namespace Raito::Renderer::OpenGL::DeferredPlus
{
	namespace
	{
		u32 g_DownSampledDepth;

		OpenGLFrameBuffer* g_FrameBuffer = nullptr;
	}


	bool Initialize()
	{
		glGenTextures(1, &g_DownSampledDepth);
		glBindTexture(GL_TEXTURE_2D, g_DownSampledDepth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1920 / 4, 1080 / 4, 0, GL_RED, GL_FLOAT, NULL);

	
		g_FrameBuffer = new  OpenGLFrameBuffer(
			FrameBufferData{
				{
					FrameBufferTextureFormat::RGBA16F,	// Position buffer
					FrameBufferTextureFormat::RGBA16F,	// Normal buffer
					FrameBufferTextureFormat::RGBA16F,	// Albedo buffer
					FrameBufferTextureFormat::RGBA16F,	// Emissive buffer
					FrameBufferTextureFormat::RGBA16F,	// RougMetalAO buffer
					FrameBufferTextureFormat::Depth		// Depth buffer
				},
				1920,
				1080,
				1
			});


		
		glBindImageTexture(1, g_DownSampledDepth, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

		return true;
	}

	void Update(const Camera& camera, const OpenGLFrameBuffer& buffer)
	{
		const auto& scene = Core::Application::Get().Scene;

		
		const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(DOWN_SAMPLE_REPROJECT));
		shader->Bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g_FrameBuffer->DepthAttachment());

		shader->SetUniformRef("u_Projection", camera.GetProjection());
		shader->SetUniformRef("u_View",   camera.GetView());
		glDispatchCompute(g_FrameBuffer->Data().Width, g_FrameBuffer->Data().Height, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		shader->UnBind();

		g_FrameBuffer->Bind();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Geometry::Update(scene, camera, *g_FrameBuffer);
		g_FrameBuffer->UnBind();



	}

	u32 GetDeferredAttachment(u32 id)
	{
		return g_FrameBuffer->ColorAttachment(id);
	}

	u32 GetDeferredDepth()
	{
		return g_DownSampledDepth;
	}

	void Shutdown()
	{
		delete g_FrameBuffer;
	}
}
