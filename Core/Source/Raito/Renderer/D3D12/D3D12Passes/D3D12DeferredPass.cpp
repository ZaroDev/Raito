#include <pch.h>
#include "D3D12DeferredPass.h"
#include <Raito/Renderer/D3D12/D3D12Core.h>

namespace Raito::Renderer::D3D12::Deferred
{
	namespace
	{
		nvrhi::FramebufferHandle g_GBuffer[c_FrameBufferCount];
	}
	bool Initialize()
	{
		for(u32 i = 0; i < c_FrameBufferCount; i++)
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

			auto posText =	Core::NVDevice()->createTexture(posDesc);
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

		std::string vertex;
		std::string pixel;
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			file.open("Shaders/D3D12/VertexShader.hlsl");

			std::stringstream stream;
			stream << file.rdbuf();

			file.close();

			vertex = stream.str();
		}
		catch (std::ifstream::failure e)
		{
			D_ERROR("Vertex shader not succesfully readed!");
		}

		try
		{
			file.open("Shaders/D3D12/PixelShader.hlsl");

			std::stringstream stream;
			stream << file.rdbuf();

			file.close();

			pixel = stream.str();
		}
		catch (std::ifstream::failure e)
		{
			D_ERROR("Vertex shader not succesfully readed!");
		}

		nvrhi::ShaderHandle vertexShader = Core::NVDevice()->createShader(
			nvrhi::ShaderDesc(nvrhi::ShaderType::Vertex),
			vertex.data(), vertex.size());

		struct Vertex {
			float position[3];
			float color[3];
		};

		nvrhi::VertexAttributeDesc attributes[] = {
			  nvrhi::VertexAttributeDesc()
				.setName("POSITION")
				.setFormat(nvrhi::Format::RGB32_FLOAT)
				.setOffset(offsetof(Vertex, position))
				.setElementStride(sizeof(Vertex)),
			nvrhi::VertexAttributeDesc()
				.setName("COLOR")
				.setFormat(nvrhi::Format::RGB32_FLOAT)
				.setOffset(offsetof(Vertex, color))
				.setElementStride(sizeof(Vertex)),
		};

		nvrhi::InputLayoutHandle inputLayout = Core::NVDevice()->createInputLayout(
			attributes, u32(std::size(attributes)), vertexShader);

		nvrhi::ShaderHandle pixelShader = Core::NVDevice()->createShader(
			nvrhi::ShaderDesc(nvrhi::ShaderType::Pixel),
			pixel.data(), pixel.size());

		auto layoutDesc = nvrhi::BindingLayoutDesc()
			.setVisibility(nvrhi::ShaderType::All)
			.addItem(nvrhi::BindingLayoutItem::VolatileConstantBuffer(0)); // constants at b0

		nvrhi::BindingLayoutHandle bindingLayout = Core::NVDevice()->createBindingLayout(layoutDesc);


		auto pipelineDesc = nvrhi::GraphicsPipelineDesc()
			.setInputLayout(inputLayout)
			.setVertexShader(vertexShader)
			.setPixelShader(pixelShader)
			.addBindingLayout(bindingLayout);

		nvrhi::GraphicsPipelineHandle graphicsPipeline = Core::NVDevice()->createGraphicsPipeline(pipelineDesc, g_GBuffer[0]);

		return true;
	}

	void Update(Renderer::Camera* camera, D3D12Surface& surface, u32 frameIndex)
	{
	}


	void Shutdown()
	{
	}
}
