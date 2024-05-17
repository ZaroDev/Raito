#include <pch.h>
#include "OpenGLCore.h"

#include <glad/glad.h>

#include "OpenGLShaderCompiler.h"
#include "GLFW/glfw3.h"

#include "OpenGLObjects/OpenGLFrameBuffer.h"
#include "OpenGLObjects/OpenGLShader.h"
#include "Window/Window.h"

#include <Raito/Time/Time.h>

#include <Renderer/Camera.h>

#include <ECS/Entity.h>

#include "Assets/Texture.h"
#include "OpenGLObjects/OpenGLMaterial.h"

#include <Raito/Renderer/OpenGL/OpenGLPasses/OpenGLPostProcessPass.h>
#include <Raito/Renderer/OpenGL/OpenGLPasses/OpenGLForwardPass.h>
#include <Raito/Renderer/OpenGL/OpenGLPasses/OpenGLDeferredPass.h>


#include "Assets/Mesh.h"

#include "optick/include/optick.h"

namespace Raito::Renderer::OpenGL
{
	namespace
	{
		Camera g_Camera(45.0, 0.1f, 1000.0f);


		std::vector<OpenGLFrameBuffer>	g_Surfaces{};
		std::vector<OpenGLMeshData>		g_Meshes{};
		std::vector<TextureData>		g_Textures{};
		std::vector<OpenGLMaterial>		g_Materials{};

		LightTechnique g_Technique = LightTechnique::Deferred;

		void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
		{
			switch (severity)
			{
			case GL_DEBUG_SEVERITY_HIGH:         O_ERROR("{0}", message); break;
			case GL_DEBUG_SEVERITY_MEDIUM:       O_WARN("{0}", message); break;
			case GL_DEBUG_SEVERITY_LOW:          O_WARN("{0}", message); break;
			case GL_DEBUG_SEVERITY_NOTIFICATION: O_LOG("{0}", message); break;
			}
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
		if (g_Technique != LightTechnique::Deferred)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_LINE_SMOOTH);
		}
		glEnable(GL_DEPTH_TEST);


		if (!ShaderCompiler::Initialize())
		{
			O_ERROR("Couldn't initalize shader compilation");
			return false;
		}

		Forward::Initialize();
		Deferred::Initialize();
		PostProcess::Initialize();


		return true;
	}

	void Shutdown()
	{
		ShaderCompiler::Shutdown();

		Forward::Shutdown();
		Deferred::Shutdown();
		PostProcess::Shutdown();

		g_Materials.clear();

		for (const auto& texture : g_Textures)
		{
			glMakeImageHandleNonResidentARB(texture.Handle);
			glDeleteTextures(1, &texture.Id);
		}
		g_Textures.clear();

		for (const auto& mesh : g_Meshes)
		{
			glDeleteBuffers(1, &mesh.VBO);
			glDeleteBuffers(1, &mesh.EBO);

			glDeleteVertexArrays(1, &mesh.VAO);
		}
		g_Meshes.clear();

	}

	Surface CreateSurface(SysWindow* window)
	{
		FrameBufferData data;

		// Deferred color attachments
		data.Attachments =
		{
			FrameBufferTextureFormat::RGBA16F,
			FrameBufferTextureFormat::Depth		// Depth buffer
		};
		data.Width = window->Info.Width;
		data.Height = window->Info.Height;
		data.Samples = 1;

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

	u32 GetColorAttachment(u32 target, u32 id)
	{
		return g_Surfaces[target].ColorAttachment(id);
	}

	u32 GetDepthAttachment(u32 id)
	{
		return g_Surfaces[id].DepthAttachment();
	}

	u32 GetDeferredBufferAttachment(u32 id)
	{
		return Deferred::GetDeferredAttachment(id);
	}

	u32 GetDeferredBufferDepth()
	{
		return Deferred::GetDeferredDepth();
	}

	void RenderSurface(u32 id)
	{
		OPTICK_EVENT();

		const OpenGLFrameBuffer& buffer = g_Surfaces[id];

		g_Camera.OnResize(buffer.Data().Width, buffer.Data().Height);
		g_Camera.OnUpdate(Time::GetDeltaTime() / 1000.0f);

		

		switch (g_Technique)
		{
			case LightTechnique::Forward: Forward::Update(&g_Camera, buffer); break;
			case LightTechnique::Deferred: Deferred::Update(&g_Camera, buffer); break;
		}

		PostProcess::Update(buffer);
	}

	u32 AddMesh(Assets::Mesh* mesh)
	{
		OpenGLMeshData data;

		switch (mesh->RenderMode)
		{
		case Assets::RenderMode::TRIANGLE: data.RenderMode = GL_TRIANGLES;
			break;
		case Assets::RenderMode::TRIANGLE_STRIP: data.RenderMode = GL_TRIANGLE_STRIP;
			break;
		case Assets::RenderMode::TRIANGLE_FAN: data.RenderMode = GL_TRIANGLE_FAN;
			break;
		}

		data.IndexCount = static_cast<u32>(mesh->Indices.size());

		glGenVertexArrays(1, &data.VAO);
		glGenBuffers(1, &data.VBO);
		glGenBuffers(1, &data.EBO);

		glBindVertexArray(data.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, data.VBO);

		glBufferData(GL_ARRAY_BUFFER, mesh->Vertices.size() * sizeof(Assets::Vertex), mesh->Vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->Indices.size() * sizeof(u32), mesh->Indices.data(), GL_STATIC_DRAW);

		// Vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Assets::Vertex), static_cast<void*>(nullptr));

		// Vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Assets::Vertex), reinterpret_cast<void*>(offsetof(Assets::Vertex, Normal)));

		// Vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Assets::Vertex), reinterpret_cast<void*>(offsetof(Assets::Vertex, TexCoords)));

		glBindVertexArray(0);

		const u32 id = static_cast<u32>(g_Meshes.size());

		g_Meshes.emplace_back(data);

		return id;
	}

	const OpenGLMeshData& GetMesh(u32 id)
	{
		return g_Meshes[id];
	}

	void RemoveMesh(u32 id)
	{
		const OpenGLMeshData& data = g_Meshes[id];

		glDeleteBuffers(1, &data.VBO);
		glDeleteBuffers(1, &data.EBO);

		glDeleteVertexArrays(1, &data.VAO);

		g_Meshes[id] = {};
	}

	u32 AddTexture(Assets::Texture* texture, ubyte* data)
	{
		TextureData textureData{};

		glGenTextures(1, &textureData.Id);
		glBindTexture(GL_TEXTURE_2D, textureData.Id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Get the texture format based on type and components
		GLenum format = GL_RGB, internalFormat = GL_RGB;
		switch (texture->ComponentsNum)
		{
		case 1:
		{
			format = internalFormat = GL_RED;
		}break;
		case 3:
		{
			format = internalFormat = GL_RGB;

			if (texture->Type == Assets::DIFFUSE)
			{
				internalFormat = GL_SRGB;
			}
			else if(texture->Type == Assets::HDR)
			{
				internalFormat = GL_RGB16F;
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}
		}break;
		case 4:
		{
			format = internalFormat = GL_RGBA;
			if (texture->Type == Assets::DIFFUSE)
			{
				internalFormat = GL_SRGB_ALPHA;
			}
		}
		break;
		}


		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, texture->Width, texture->Height, 0, format, GL_UNSIGNED_BYTE, data);

		if (texture->Type != Assets::HDR)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		// Get and load into VRAM using bindless textures
		// TODO: Unload textures that aren't used in the scene
		textureData.Handle = glGetTextureHandleARB(textureData.Id);
		glMakeTextureHandleResidentARB(textureData.Handle);

		texture->RenderData.RenderId = textureData.Id;
		texture->RenderData.Handle = textureData.Handle;

		g_Textures.emplace_back(textureData);

		return static_cast<u32>(g_Textures.size()) - 1;
	}

	void RemoveTexture(u32 id)
	{
		glMakeImageHandleNonResidentARB(g_Textures[id].Handle);
		glDeleteTextures(1, &g_Textures[id].Id);
		g_Textures[id] = {};
	}

	u32 AddMaterial(EngineShader shaderId)
	{
		g_Materials.emplace_back(shaderId);
		return static_cast<u32>(g_Materials.size()) - 1;
	}

	void SetMaterialValue(u32 id, const char* name, ubyte* data, size_t size)
	{
		g_Materials[id].SetValuePtr(name, data, size);
	}

	OpenGLMaterial& GetMaterial(u32 id)
	{
		return g_Materials[id];
	}

	void RemoveMaterial(u32 id)
	{
		g_Materials[id] = OpenGLMaterial(-1);
	}
}
