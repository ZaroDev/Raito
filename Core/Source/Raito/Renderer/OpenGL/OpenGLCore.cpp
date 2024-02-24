/*
MIT License

Copyright (c) 2023 Víctor Falcón Zaro

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
#include <Core/Application.h>

#include "Assets/Texture.h"
#include "OpenGLObjects/OpenGLMaterial.h"

namespace Raito::Renderer::OpenGL
{
	namespace
	{
		Camera g_Camera(45.0, 0.1f, 1000.0f);

		std::vector<OpenGLFrameBuffer> g_Surfaces{};

		struct OpenGLMeshData
		{
			u32 VAO = 0;
			u32 VBO = 0;
			u32 EBO = 0;

			GLsizei IndexCount = 0;
		};


		std::vector<OpenGLMeshData> g_Meshes{};
		std::vector<u32> g_Textures{};
		std::vector<OpenGLMaterial> g_Materials{};


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

		g_Camera.OnResize(buffer.Data().Width, buffer.Data().Height);
		g_Camera.OnUpdate(Time::GetDeltaTime() / 1000.0f);


		buffer.Bind();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		// Geometry pass
		{

			auto& scene = Core::Application::Get().Scene;
			const auto view = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::MeshComponent>();

			for (auto& entity : view)
			{
				const ECS::MeshComponent& mesh = view.get<ECS::MeshComponent>(entity);
				const OpenGLMeshData& meshData = g_Meshes[mesh.MeshId];
				const Mat4 model = view.get<ECS::TransformComponent>(entity).GetTransform();
				
				OpenGLMaterial& material = g_Materials[mesh.MaterialId];

				material.SetValue("u_View", g_Camera.GetView());
				material.SetValue("u_Projection", g_Camera.GetProjection());
				material.SetValue("u_Model", model);
				material.SetValue("u_ObjectColor", V3(0.9f, 0.9f, 0.9f));
				material.SetValue("u_LightColor", V3(1.0f));

				material.Bind();


				glBindVertexArray(meshData.VAO);
				glDrawElements(GL_TRIANGLES, meshData.IndexCount, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				material.UnBind();
			}
		}


		buffer.UnBind();


		// Post-processing pass
		{
			// Clear the back buffer
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::POST_PROCESS));

			shader->Bind();

			glBindVertexArray(g_FrameBufferQuadVAO);
			glDisable(GL_DEPTH_TEST);
			glBindTexture(GL_TEXTURE_2D, buffer.ColorAttachment());
			glDrawArrays(GL_TRIANGLES, 0, 6);

			shader->UnBind();
		}

	}

	u32 AddMesh(Assets::Mesh* mesh)
	{
		OpenGLMeshData data;

		data.IndexCount = (u32)mesh->Indices.size();

		glGenVertexArrays(1, &data.VAO);
		glGenBuffers(1, &data.VBO);
		glGenBuffers(1, &data.EBO);

		glBindVertexArray(data.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, data.VBO);

		glBufferData(GL_ARRAY_BUFFER, mesh->Vertices.size() * sizeof(Assets::Vertex), &mesh->Vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->Indices.size() * sizeof(u32), &mesh->Indices[0], GL_STATIC_DRAW);

		// Vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Assets::Vertex), (void*)0);

		// Vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Assets::Vertex), (void*)offsetof(Assets::Vertex, Normal));

		// Vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Assets::Vertex), (void*)offsetof(Assets::Vertex, TexCoords));

		glBindVertexArray(0);

		const u32 id = (u32)g_Meshes.size();

		g_Meshes.emplace_back(data);

		return id;
	}

	void RemoveMesh(u32 id)
	{
		const OpenGLMeshData& data = g_Meshes[id];

		glDeleteBuffers(1, &data.VBO);
		glDeleteBuffers(1, &data.EBO);

		glDeleteVertexArrays(1, &data.VAO);

		g_Meshes[id] = OpenGLMeshData();
	}

	u32 AddTexture(Assets::Texture* texture, ubyte* data)
	{
		u32 textureId;

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->Width, texture->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		texture->RenderId = textureId;

		g_Textures.emplace_back(textureId);
		
		return (u32)g_Textures.size() - 1;
	}

	void RemoveTexture(u32 id)
	{
		glDeleteTextures(1, &g_Textures[id]);
		g_Textures[id] = U32_MAX;
	}

	u32 AddMaterial(EngineShader shaderId)
	{
		g_Materials.emplace_back(shaderId);
		return (u32)g_Materials.size() - 1;
	}

	void SetMaterialValue(u32 id, const char* name, ubyte* data, size_t size)
	{
		g_Materials[id].SetValuePtr(name, data, size);
	}

	void RemoveMaterial(u32 id)
	{
		g_Materials.erase(g_Materials.begin() + id);
	}
}
