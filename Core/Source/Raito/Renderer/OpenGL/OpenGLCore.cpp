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

#include "Assets/AssetImport.h"
#include "Assets/Texture.h"
#include "OpenGLObjects/OpenGLMaterial.h"
#include "optick/include/optick.h"

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


		OpenGLMeshData g_LightSphereData;
		OpenGLMeshData g_CubeData;

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

			for (u32 i = 0; i < g_BloomBuffers.size(); i++)
			{
				g_BloomBuffers[i] = new OpenGLFrameBuffer{ data };
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

		void CreateCube()
		{
			OpenGLMeshData cube;

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
			glGenVertexArrays(1, &cube.VAO);
			glGenBuffers(1, &cube.VBO);
			// fill buffer
			glBindBuffer(GL_ARRAY_BUFFER, cube.VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			// link vertex attributes
			glBindVertexArray(cube.VAO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			g_CubeData = cube;
		}

		void RenderCube()
		{
			glBindVertexArray(g_CubeData.VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}

		void CreateSphere()
		{
			OpenGLMeshData sphere;
			glGenVertexArrays(1, &sphere.VAO);

			glGenBuffers(1, &sphere.VBO);
			glGenBuffers(1, &sphere.EBO);

			std::vector<glm::vec3> positions;
			std::vector<glm::vec2> uv;
			std::vector<glm::vec3> normals;
			std::vector<unsigned int> indices;

			const u32 X_SEGMENTS = 64;
			const u32 Y_SEGMENTS = 64;
			const float PI = 3.14159265359f;
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
			{
				for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
				{
					float xSegment = (float)x / (float)X_SEGMENTS;
					float ySegment = (float)y / (float)Y_SEGMENTS;
					float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
					float yPos = std::cos(ySegment * PI);
					float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

					positions.push_back(glm::vec3(xPos, yPos, zPos));
					uv.push_back(glm::vec2(xSegment, ySegment));
					normals.push_back(glm::vec3(xPos, yPos, zPos));
				}
			}

			bool oddRow = false;
			for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
			{
				if (!oddRow) // even rows: y == 0, y == 2; and so on
				{
					for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
					{
						indices.push_back(y * (X_SEGMENTS + 1) + x);
						indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					}
				}
				else
				{
					for (int x = X_SEGMENTS; x >= 0; --x)
					{
						indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
						indices.push_back(y * (X_SEGMENTS + 1) + x);
					}
				}
				oddRow = !oddRow;
			}
			sphere.IndexCount = static_cast<unsigned int>(indices.size());

			std::vector<float> data;
			for (unsigned int i = 0; i < positions.size(); ++i)
			{
				data.push_back(positions[i].x);
				data.push_back(positions[i].y);
				data.push_back(positions[i].z);
				if (!normals.empty())
				{
					data.push_back(normals[i].x);
					data.push_back(normals[i].y);
					data.push_back(normals[i].z);
				}
				if (!uv.empty())
				{
					data.push_back(uv[i].x);
					data.push_back(uv[i].y);
				}
			}
			glBindVertexArray(sphere.VAO);
			glBindBuffer(GL_ARRAY_BUFFER, sphere.VBO);
			glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere.EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
			constexpr u32 stride = (3 + 2 + 3) * sizeof(float);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, static_cast<void*>(0));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));

			g_LightSphereData = sphere;
		}

		void RenderSphere()
		{
			glBindVertexArray(g_LightSphereData.VAO);
			glDrawElements(GL_TRIANGLE_STRIP, g_LightSphereData.IndexCount, GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
		}

		u32 g_IrradianceCubeMap = 0;
		u64 g_IrradianceHandler = 0;
		u32 g_EnvironmentCubeMap = 0;
		u32 g_CaptureFBO, g_CaptureRBO;

		void InitializeImageBasedLighting()
		{
			ImportTexture("Textures/thatch_chapel_4k.hdr", Assets::HDR);
			Assets::Texture* hdri = Assets::GetTexture("Textures/thatch_chapel_4k.hdr");

			if(!hdri)
			{
				return;
			}
			// Create the framebuffer to store the views
			glGenFramebuffers(1, &g_CaptureFBO);
			glGenRenderbuffers(1, &g_CaptureRBO);
			glBindFramebuffer(GL_FRAMEBUFFER, g_CaptureFBO);
			glBindRenderbuffer(GL_RENDERBUFFER, g_CaptureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_CaptureRBO);


			glGenTextures(1, &g_EnvironmentCubeMap);
			glBindTexture(GL_TEXTURE_CUBE_MAP, g_EnvironmentCubeMap);
			for (unsigned int i = 0; i < 6; ++i)
			{
				// note that we store each face with 16 bit floating point values
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB,
					512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


			Mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
			Mat4 captureViews[] =
			{
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
			};

			{
				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::EQUIRECTANGULAR_TO_CUBEMAP));

				shader->Bind();

				glUniformHandleui64ARB(shader->GetUniformLocation("u_EquirectangularMap"), hdri->RenderData.Handle);
				shader->SetUniformRef("u_Projection", captureProjection);

				glViewport(0, 0, 512, 512);
				glBindFramebuffer(GL_FRAMEBUFFER, g_CaptureFBO);
				for(u32 i = 0; i < 6; i++)
				{
					shader->SetUniformRef("u_View", captureViews[i]);

					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, g_EnvironmentCubeMap, 0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					RenderCube();
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				shader->UnBind();
			}

			glGenTextures(1, &g_IrradianceCubeMap);
			glBindTexture(GL_TEXTURE_CUBE_MAP, g_IrradianceCubeMap);

			for(u32 i = 0; i < 6; i++)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


			glBindFramebuffer(GL_FRAMEBUFFER, g_CaptureFBO);
			glBindRenderbuffer(GL_RENDERBUFFER, g_CaptureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
			{
				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::IRRADIANCE));

				shader->Bind();

				shader->SetUniform("u_EnivornmentMap", 0);
				shader->SetUniformRef("u_Projection", captureProjection);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, g_EnvironmentCubeMap);

				glViewport(0, 0, 32, 32);
				glBindFramebuffer(GL_FRAMEBUFFER, g_CaptureFBO);
				for (u32 i = 0; i < 6; i++)
				{
					shader->SetUniformRef("u_View", captureViews[i]);

					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, g_IrradianceCubeMap, 0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					RenderCube();
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				shader->UnBind();
			}

			g_IrradianceHandler = glGetTextureHandleARB(g_IrradianceCubeMap);
			glMakeTextureHandleResidentARB(g_IrradianceHandler);

			{
				const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::SKYBOX));
				shader->Bind();
				shader->SetUniform("u_EnvironmentMap", 0);
				shader->UnBind();
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
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);

		CreateSphere();
		CreateCube();

		if (!ShaderCompiler::Initialize())
		{
			O_ERROR("Couldn't initalize shader compilation");
			return false;
		}

		InitializePostProcessPass();
		InitializeBloomPass();
		InitializeImageBasedLighting();


		return true;
	}

	void Shutdown()
	{
		ShaderCompiler::Shutdown();
		ShutdownPostProcessPass();

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
		OPTICK_EVENT();

		const OpenGLFrameBuffer& buffer = g_Surfaces[id];

		g_Camera.OnResize(buffer.Data().Width, buffer.Data().Height);
		g_Camera.OnUpdate(Time::GetDeltaTime() / 1000.0f);


		buffer.Bind();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		// Geometry pass
		{

			auto& scene = Core::Application::Get().Scene;
			const auto view = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::MeshComponent>();
			const auto lightView = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::LightComponent>();
			for (auto& entity : view)
			{
				const ECS::MeshComponent& mesh = view.get<ECS::MeshComponent>(entity);
				const OpenGLMeshData& meshData = g_Meshes[mesh.MeshId];
				const Mat4 model = view.get<ECS::TransformComponent>(entity).GetTransform();
				const Mat3 normalMatrix = glm::transpose(glm::inverse(Mat3(model)));

				OpenGLMaterial& material = g_Materials[mesh.MaterialId];


				material.SetValue("u_ViewPos", g_Camera.GetPosition());
				for (auto& light : lightView)
				{
					material.SetValue("u_LightPosition", lightView.get<ECS::TransformComponent>(light).Translation);
					material.SetValue("u_LightColor", lightView.get<ECS::LightComponent>(light).Color);
				}

				material.SetValue("u_View", g_Camera.GetView());
				material.SetValue("u_Projection", g_Camera.GetProjection());
				material.SetValue("u_Model", model);
				material.SetValue("u_NormalMatrix", normalMatrix);
				material.SetValue("u_IrradianceMap", TextureData{ g_IrradianceHandler, g_IrradianceCubeMap });
				
				material.Bind();

				glBindVertexArray(meshData.VAO);
				glDrawElements(GL_TRIANGLES, meshData.IndexCount, GL_UNSIGNED_INT, nullptr);
				glBindVertexArray(0);

				material.UnBind();
			}
			// Render lights
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::DEFAULT_LIGHT));
			shader->Bind();
			for(auto& light : lightView)
			{
				const auto& transform = lightView.get<ECS::TransformComponent>(light).GetTransform();
				const V3& lightColor = lightView.get<ECS::LightComponent>(light).Color;

				shader->SetUniformRef("u_View", g_Camera.GetView());
				shader->SetUniformRef("u_Projection", g_Camera.GetProjection());
				shader->SetUniformRef("u_Model", transform);

				shader->SetUniformRef("u_Color", lightColor);

				RenderSphere();
			}
			shader->UnBind();
		}

		// Render skybox
		{
			const auto shader = dynamic_cast<OpenGLShader*>(ShaderCompiler::GetShaderWithEngineId(EngineShader::SKYBOX));
			shader->Bind();
			shader->SetUniform("u_EnvironmentMap", 0);

			shader->SetUniformRef("u_View", g_Camera.GetView());
			shader->SetUniformRef("u_Projection", g_Camera.GetProjection());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, g_EnvironmentCubeMap);

			RenderCube();

			shader->UnBind();
		}


		buffer.UnBind();




		bool horizontal = true;
		// Bloom pass
		// TODO: Make Physically based bloom instead of the crappy learnopengl.com implementation
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

	void RemoveMaterial(u32 id)
	{
		g_Materials[id] = OpenGLMaterial(-1);
	}
}
