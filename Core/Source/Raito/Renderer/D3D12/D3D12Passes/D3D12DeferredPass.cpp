#include <pch.h>
#include "D3D12DeferredPass.h"
#include <Raito/Renderer/D3D12/D3D12Core.h>

#include "nvrhi/utils.h"
#include "Renderer/D3D12/D3D12Shaders.h"
#include "Renderer/D3D12/D3D12Objects/D3D12Command.h"

namespace Raito::Renderer::D3D12::Deferred
{
	namespace
	{
		nvrhi::FramebufferHandle g_GBuffer[c_FrameBufferCount];
		struct Vertex {
			float position[4];
			float color[4];
		};
		Vertex vertex_data_array[] = {
			  { { 0.0f, 0.25f * 16/9 , 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * 16 / 9, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * 16 / 9, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};
		nvrhi::GraphicsPipelineHandle graphicsPipeline;
		nvrhi::BufferHandle vertexBuffer;
	}
	bool Initialize()
	{
		for (u32 i = 0; i < c_FrameBufferCount; i++)
		{
			auto posDesc = nvrhi::TextureDesc()
				.setDimension(nvrhi::TextureDimension::Texture2D)
				.setFormat(nvrhi::Format::RGBA8_UNORM)
				.setWidth(1920)
				.setHeight(1080)
				.setIsRenderTarget(true)
				.setDebugName("Deferred position");

			auto normalDesc = nvrhi::TextureDesc()
				.setDimension(nvrhi::TextureDimension::Texture2D)
				.setFormat(nvrhi::Format::RGBA8_UNORM)
				.setWidth(1920)
				.setHeight(1080)
				.setIsRenderTarget(true)
				.setDebugName("Deferred normal");


			auto albedoDesc = nvrhi::TextureDesc()
				.setDimension(nvrhi::TextureDimension::Texture2D)
				.setFormat(nvrhi::Format::RGBA8_UNORM)
				.setWidth(1920)
				.setHeight(1080)
				.setIsRenderTarget(true)
				.setDebugName("Deferred albedo");

			auto depthDesc = nvrhi::TextureDesc()
				.setDimension(nvrhi::TextureDimension::Texture2D)
				.setFormat(nvrhi::Format::D32)
				.setWidth(1920)
				.setHeight(1080)
				.setIsRenderTarget(true)
				.setDebugName("Deferred depth");

			auto posText = Core::NVDevice()->createTexture(posDesc);
			auto normalText = Core::NVDevice()->createTexture(normalDesc);
			auto albedoText = Core::NVDevice()->createTexture(albedoDesc);
			auto depthText = Core::NVDevice()->createTexture(depthDesc);

			const auto gBufferDesc = nvrhi::FramebufferDesc()
				.addColorAttachment(posText)
				.addColorAttachment(normalText)
				.addColorAttachment(albedoText)
				.setDepthAttachment(depthText);

			g_GBuffer[i] = Core::NVDevice()->createFramebuffer(gBufferDesc);
		}

		auto vertex = Shaders::GetEngineShader(Shaders::EngineShader::FULLSCREEN_TRIANGLE);

		nvrhi::ShaderHandle vertexShader = Core::NVDevice()->createShader(
			nvrhi::ShaderDesc(nvrhi::ShaderType::Vertex),
			vertex.pShaderBytecode, vertex.BytecodeLength);

	

		nvrhi::VertexAttributeDesc attributes[] = {
			  nvrhi::VertexAttributeDesc()
				.setName("POSITION")
				.setFormat(nvrhi::Format::RGBA32_FLOAT)
				.setOffset(offsetof(Vertex, position))
				.setElementStride(sizeof(Vertex)),
			nvrhi::VertexAttributeDesc()
				.setName("COLOR")
				.setFormat(nvrhi::Format::RGBA32_FLOAT)
				.setOffset(offsetof(Vertex, color))
				.setElementStride(sizeof(Vertex)),
		};

		nvrhi::InputLayoutHandle inputLayout = Core::NVDevice()->createInputLayout(
			attributes, 2, vertexShader);

		auto pixel = Shaders::GetEngineShader(Shaders::EngineShader::FILL_COLOR_PS);
		nvrhi::ShaderHandle pixelShader = Core::NVDevice()->createShader(
			nvrhi::ShaderDesc(nvrhi::ShaderType::Pixel),
			pixel.pShaderBytecode, pixel.BytecodeLength);

		auto layoutDesc = nvrhi::BindingLayoutDesc()
			.setVisibility(nvrhi::ShaderType::All)
			.addItem(nvrhi::BindingLayoutItem::VolatileConstantBuffer(0)); // constants at b0

		nvrhi::BindingLayoutHandle bindingLayout = Core::NVDevice()->createBindingLayout(layoutDesc);


		auto pipelineDesc = nvrhi::GraphicsPipelineDesc()
			.setInputLayout(inputLayout)
			.setVertexShader(vertexShader)
			.setPixelShader(pixelShader)
			.addBindingLayout(bindingLayout);

		graphicsPipeline = Core::NVDevice()->createGraphicsPipeline(pipelineDesc, g_GBuffer[0]);

		

		auto vertexBufferDesc = nvrhi::BufferDesc()
			.setByteSize(sizeof(vertex_data_array))
			.setIsVertexBuffer(true)
			.setInitialState(nvrhi::ResourceStates::VertexBuffer)
			.setKeepInitialState(true)
			.setDebugName("Triangle buffer");


		vertexBuffer = Core::NVDevice()->createBuffer(vertexBufferDesc);

		const nvrhi::CommandListHandle commandList = Core::GraphicsCommand().CommandList();
		commandList->open();
		commandList->writeBuffer(vertexBuffer, vertex_data_array, sizeof(vertex_data_array));
		commandList->close();
		Core::NVDevice()->executeCommandList(commandList);

		return true;
	}

	void Update(Renderer::Camera* camera, D3D12Surface& surface, u32 frameIndex)
	{
		auto frameBuffer = surface.FrameBuffer(frameIndex);

		auto commandList = Core::GraphicsCommand().CommandList();

		nvrhi::utils::ClearColorAttachment(commandList, frameBuffer, 0, nvrhi::Color{ 1.0f });

		auto buffer = nvrhi::VertexBufferBinding{ vertexBuffer };
		auto graphicsState = nvrhi::GraphicsState()
			.setPipeline(graphicsPipeline)
			.setFramebuffer(frameBuffer)
			.setViewport(nvrhi::ViewportState().addViewportAndScissorRect(nvrhi::Viewport(surface.Width(), surface.Height())))
			.addVertexBuffer(buffer);

		commandList->setGraphicsState(graphicsState);

		auto drawArguments = nvrhi::DrawArguments()
			.setVertexCount(std::size(vertex_data_array));

		commandList->draw(drawArguments);
	}


	void Shutdown()
	{
	}
}
