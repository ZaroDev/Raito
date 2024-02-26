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
		std::array<OpenGLFrameBuffer*, 2> g_BloomBuffers{};

		struct OpenGLMeshData
		{
			u32 VAO = 0;
			u32 VBO = 0;
			u32 EBO = 0;

			GLsizei IndexCount = 0;
		};

		

		std::vector<OpenGLMeshData> g_Meshes{};
		std::vector<TextureData> g_Textures{};
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

		void InitializeBloomPass()
		{
			SysWindow window = Window::GetWindow();
			FrameBufferData data;
			data.Attachments = { FrameBufferTextureFormat::RGBA16F };
			data.Width = window.Info.Width;
			data.Height = window.Info.Height;
			data.Samples = 1;
			data.SwapChainTarget = false;

			for(u32 i = 0; i < g_BloomBuffers.size(); i++)
			{
				g_BloomBuffers[i] = new OpenGLFrameBuffer{data};
			}

			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::POST_PROCESS));
			shader->Bind();
			shader->SetUniform("u_ScreenTexture", 0);
			shader->SetUniform("u_BloomTexture", 1);
			shader->UnBind();
		}

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
		InitializeBloomPass();

		return true;
	}

	void Shutdown()
	{
		ShaderCompiler::Shutdown();
		ShutdownPostProcessPass();

		g_Materials.clear();

		for (const auto & texture : g_Textures)
		{
			glMakeImageHandleNonResidentARB(texture.Handle);
			glDeleteTextures(1, &texture.Id);
		}
		g_Textures.clear();

		for (const auto & mesh : g_Meshes)
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
		data.Attachments = 
		{
			FrameBufferTextureFormat::RGBA16F,	// Normal color buffer
			FrameBufferTextureFormat::RGBA16F,	// Bloom color buffer
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

		for (const auto& buffer : g_BloomBuffers)
		{
			buffer->Resize(width, height);
		}
	}

	u32 SurfaceWidth(u32 id)
	{
		return g_Surfaces[id].Data().Width;
	}

	u32 SurfaceHeight(u32 id)
	{
		return g_Surfaces[id].Data().Height;
	}

	u32 GetColorGetAttachment(u32 target, u32 id)
	{
		return g_Surfaces[target].ColorAttachment(id);
	}

	u32 GetDepthAttachment(u32 id)
	{
		return g_Surfaces[id].DepthAttachment();
	}

	void RenderSurface(u32 id)
	{
		const OpenGLFrameBuffer& buffer = g_Surfaces[id];

		g_Camera.OnResize(buffer.Data().Width, buffer.Data().Height);
		g_Camera.OnUpdate(Time::GetDeltaTime() / 1000.0f);


		buffer.Bind();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
				material.SetValue("u_ViewPos", g_Camera.GetPosition());
				material.SetValue("u_Projection", g_Camera.GetProjection());
				material.SetValue("u_Model", model);
				material.SetValue("u_ObjectColor", V3(0.9f, 0.9f, 0.9f));
				material.SetValue("u_LightColor", V3(1.0f));

				material.Bind();

				glBindVertexArray(meshData.VAO);
				glDrawElements(GL_TRIANGLES, meshData.IndexCount, GL_UNSIGNED_INT, nullptr);
				glBindVertexArray(0);

				material.UnBind();
			}
		}
		buffer.UnBind();


		bool horizontal = true;
		// Bloom pass
		{
			constexpr u32 blurAmount = 10u;

			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::GAUSSIAN_BLUR));
			shader->Bind();


			for (u32 i = 0; i < blurAmount; i++)
			{
				g_BloomBuffers[horizontal]->Bind();
				shader->SetUniform("u_Horizontal", horizontal);

				const u32 blurTexture = i == 0 ? buffer.ColorAttachment(1) : g_BloomBuffers[!horizontal]->ColorAttachment();

				glBindVertexArray(g_FrameBufferQuadVAO);
				glDisable(GL_DEPTH_TEST);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, blurTexture);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				horizontal = !horizontal;
			}
			
			shader->UnBind();
			g_BloomBuffers[!horizontal]->UnBind();
		}


		// Post-processing pass
		{
			// Clear the back buffer
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::POST_PROCESS));

			shader->Bind();

			glBindVertexArray(g_FrameBufferQuadVAO);
			glDisable(GL_DEPTH_TEST);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, buffer.ColorAttachment());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, g_BloomBuffers[!horizontal]->ColorAttachment());

			glDrawArrays(GL_TRIANGLES, 0, 6);

			shader->UnBind();
		}

	}

	u32 AddMesh(Assets::Mesh* mesh)
	{
		OpenGLMeshData data;

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

		glTexImage2D(GL_TEXTURE_2D, 0, texture->Type == Assets::DIFFUSE ? GL_SRGB : GL_RGB, texture->Width, texture->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Get and load into VRAM using bindless textures
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

	void RemoveMaterial(u32 id)
	{
		g_Materials[id] = OpenGLMaterial(-1);
	}
}
