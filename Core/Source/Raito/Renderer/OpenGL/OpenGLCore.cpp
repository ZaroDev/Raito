#include <pch.h>
#include "OpenGLCore.h"

#include <glad/glad.h>

#include "OpenGLShaderCompiler.h"
#include "GLFW/glfw3.h"


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
#include <Raito/Renderer/OpenGL/OpenGLPasses/OpenGLShadowPass.h>


#include "Assets/Mesh.h"
#include "OpenGLPasses/OpenGLDeferredPlus.h"
#include "OpenGLPasses/OpenGLSkyboxPass.h"

#include "optick/include/optick.h"

namespace Raito::Renderer::OpenGL
{
	namespace
	{
		u32 g_FrameBufferQuadVAO, g_FrameBufferQuadVBO;
		u32 g_CubeVAO, g_CubeVBO;

		bool g_EnableParallax = true;

		std::vector<OpenGLFrameBuffer>	g_Surfaces{};
		std::vector<OpenGLMeshData>		g_Meshes{};
		std::vector<TextureData>		g_Textures{};
		std::vector<OpenGLMaterial>		g_Materials{};

		LightTechnique g_Technique = LightTechnique::DeferredPlus;

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

		Skybox::Initialize();
		Shadows::Initialize();
		Forward::Initialize();
		Deferred::Initialize();
		DeferredPlus::Initialize();
		PostProcess::Initialize();



		return true;
	}

	void RenderSurface(u32 id)
	{
		auto& camera = GetMainCamera();
		OPTICK_CATEGORY("Update Renderer", Optick::Category::Rendering);

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, 1920, 1080);

		const OpenGLFrameBuffer& buffer = g_Surfaces[id];

		camera.OnResize(buffer.Data().Width, buffer.Data().Height);
		camera.OnUpdate(Time::GetDeltaTime() / 1000.0f);


		Shadows::Update(&camera);

		switch (g_Technique)
		{
		case LightTechnique::Forward: Forward::Update(&camera, buffer); break;
		case LightTechnique::Deferred: Deferred::Update(camera, buffer); break;
		case LightTechnique::DeferredPlus: DeferredPlus::Update(camera, buffer); break;
		}
		Skybox::Update(&camera);
		PostProcess::Update(buffer);
	}

	void Shutdown()
	{
		ShaderCompiler::Shutdown();
		Skybox::Shutdown();
		Shadows::Shutdown();
		Forward::Shutdown();
		Deferred::Shutdown();
		DeferredPlus::Shutdown();
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


		glDeleteVertexArrays(1, &g_FrameBufferQuadVAO);
		glDeleteBuffers(1, &g_FrameBufferQuadVBO);
	}

	void RenderFullScreenQuad()
	{
		glBindVertexArray(g_FrameBufferQuadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

	}

	Surface CreateSurface(SysWindow* window)
	{
		FrameBufferData data;

		// Deferred color attachments
		data.Attachments =
		{
			FrameBufferTextureFormat::RGBA16F,	// Color buffer
			FrameBufferTextureFormat::RGBA16F,	// Bloom buffer
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
		return g_Technique == LightTechnique::Deferred ? Deferred::GetDeferredAttachment(id) : DeferredPlus::GetDeferredAttachment(id);
	}

	u32 GetDeferredBufferDepth()
	{
		return  g_Technique == LightTechnique::Deferred ?  Deferred::GetDeferredDepth() : DeferredPlus::GetDeferredDepth();
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

		data.AABB = mesh->AABB;
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
		// Vertex tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Assets::Vertex), reinterpret_cast<void*>(offsetof(Assets::Vertex, Tangent)));

		// Vertex Bitangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Assets::Vertex), reinterpret_cast<void*>(offsetof(Assets::Vertex, BiTangent)));

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
			else if (texture->Type == Assets::HDR)
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
		if (g_Textures.empty())
		{
			return;
		}
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


	void EnableParallax(bool value)
	{
		g_EnableParallax = value;
	}

	bool IsParallaxEnabled()
	{
		return g_EnableParallax;
	}

	void RenderCube()
	{
		// initialize (if necessary)
		if (g_CubeVAO == 0)
		{
			float vertices[] = {
				// back face
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
				// front face
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				// left face
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				// right face
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
				 // bottom face
				 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
				  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				 // top face
				 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
				  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
			};
			glGenVertexArrays(1, &g_CubeVAO);
			glGenBuffers(1, &g_CubeVBO);
			// fill buffer
			glBindBuffer(GL_ARRAY_BUFFER, g_CubeVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			// link vertex attributes
			glBindVertexArray(g_CubeVAO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
		// render Cube
		glBindVertexArray(g_CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
}
