#include <pch.h>
#include "D3D12DeferredPass.h"
#include <Raito/Renderer/D3D12/D3D12Core.h>

#include "Assets/Mesh.h"
#include "ECS/Components.h"
#include "nvrhi/utils.h"
#include "Renderer/D3D12/D3D12Shaders.h"
#include "Renderer/D3D12/D3D12Objects/D3D12Command.h"

#include <Raito/Core/Application.h>

#include "Renderer/D3D12/D3D12ContentUploader.h"


namespace Raito::Renderer::D3D12::Deferred
{
	namespace
	{
		nvrhi::FramebufferHandle g_GBuffer[c_FrameBufferCount];
		
		nvrhi::GraphicsPipelineHandle graphicsPipeline;
		nvrhi::BindingLayoutHandle bindingLayout;
		nvrhi::BufferHandle constantBuffer;
		nvrhi::SamplerHandle sampler;
		nvrhi::BindingSetHandle bindingSet;

		struct CBuffer
		{
			Mat4 MVP;
			Mat4 Model;
		};
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

		auto vertex = Shaders::GetEngineShader(Shaders::EngineShader::VERTEX_SHADER);

		nvrhi::ShaderHandle vertexShader = Core::NVDevice()->createShader(
			nvrhi::ShaderDesc(nvrhi::ShaderType::Vertex),
			vertex.pShaderBytecode, vertex.BytecodeLength);

		nvrhi::VertexAttributeDesc attributes[] = {
			  nvrhi::VertexAttributeDesc()
				.setName("POSITION")
				.setFormat(nvrhi::Format::RGB32_FLOAT)
				.setOffset(offsetof(Assets::Vertex, Position))
				.setElementStride(sizeof(Assets::Vertex)),
			nvrhi::VertexAttributeDesc()
				.setName("NORMAL")
				.setFormat(nvrhi::Format::RGB32_FLOAT)
				.setOffset(offsetof(Assets::Vertex, Normal))
				.setElementStride(sizeof(Assets::Vertex)),
			nvrhi::VertexAttributeDesc()
				.setName("TEXCOORD")
				.setFormat(nvrhi::Format::RG32_FLOAT)
				.setOffset(offsetof(Assets::Vertex, TexCoords))
				.setElementStride(sizeof(Assets::Vertex)),
		};

		nvrhi::InputLayoutHandle inputLayout = Core::NVDevice()->createInputLayout(
			attributes, std::size(attributes), vertexShader);

		auto pixel = Shaders::GetEngineShader(Shaders::EngineShader::PIXEL_SHADER);
		nvrhi::ShaderHandle pixelShader = Core::NVDevice()->createShader(
			nvrhi::ShaderDesc(nvrhi::ShaderType::Pixel),
			pixel.pShaderBytecode, pixel.BytecodeLength);

		auto layoutDesc = nvrhi::BindingLayoutDesc()
			.setVisibility(nvrhi::ShaderType::All)
			.addItem(nvrhi::BindingLayoutItem::Texture_SRV(0))
			.addItem(nvrhi::BindingLayoutItem::VolatileConstantBuffer(0))
			.addItem(nvrhi::BindingLayoutItem::Sampler(0));

		bindingLayout = Core::NVDevice()->createBindingLayout(layoutDesc);


		auto constantBufferDesc = nvrhi::BufferDesc()
			.setByteSize(sizeof(CBuffer))
			.setIsConstantBuffer(true)
			.setIsVolatile(true)
			.setDebugName("Constant buffer")
			.setMaxVersions(16);


		constantBuffer = Core::NVDevice()->createBuffer(constantBufferDesc);

		auto samplerDesc = nvrhi::SamplerDesc()
			.setAllAddressModes(nvrhi::SamplerAddressMode::ClampToEdge)
			.setAllFilters(true);

		sampler = Core::NVDevice()->createSampler(samplerDesc);

		auto rasterState = nvrhi::RasterState()
			.setFrontCounterClockwise(true)
			.setCullMode(nvrhi::RasterCullMode::Front);

		auto blendState = nvrhi::BlendState()
			.disableAlphaToCoverage();

		auto depthState = nvrhi::DepthStencilState()
			.enableStencil()
			.setDepthWriteEnable(true)
			.setDepthFunc(nvrhi::ComparisonFunc::LessOrEqual)
			.setStencilReadMask(0)
			.setStencilWriteMask(0)
			.setStencilRefValue(0)
			.setFrontFaceStencil(nvrhi::DepthStencilState::StencilOpDesc().setPassOp(nvrhi::StencilOp::Replace))
			.setBackFaceStencil(nvrhi::DepthStencilState::StencilOpDesc().setPassOp(nvrhi::StencilOp::Replace));

		auto renderState = nvrhi::RenderState()
			.setRasterState(rasterState)
			.setBlendState(blendState)
			.setDepthStencilState(depthState);
		
		auto pipelineDesc = nvrhi::GraphicsPipelineDesc()
			.setPrimType(nvrhi::PrimitiveType::TriangleList)
			.setInputLayout(inputLayout)
			.setVertexShader(vertexShader)
			.setPixelShader(pixelShader)
			.addBindingLayout(bindingLayout)
			.setRenderState(renderState);
		graphicsPipeline = Core::NVDevice()->createGraphicsPipeline(pipelineDesc, g_GBuffer[0]);

		return true;
	}

	void Update(Renderer::Camera* camera, D3D12Surface& surface, u32 frameIndex)
	{
		const auto& frameBuffer = surface.FrameBuffer(frameIndex);
		const auto& commandList = Core::GraphicsCommand().CommandList();

		nvrhi::utils::ClearColorAttachment(commandList, frameBuffer, 0, nvrhi::Color{ 0.0f });
		//nvrhi::utils::ClearDepthStencilAttachment(commandList, frameBuffer, 0, 0);

		auto& scene = Raito::Core::Application::Get().Scene;
		const auto view = scene.GetAllEntitiesWith<ECS::TransformComponent, ECS::MeshComponent>();

		

		static int i = 0;
		if (i == 0)
		{
			const auto bindingSetDesc = nvrhi::BindingSetDesc()
				.addItem(nvrhi::BindingSetItem::Texture_SRV(0, Upload::GetTexture(0)))
				.addItem(nvrhi::BindingSetItem::ConstantBuffer(0, constantBuffer))
				.addItem(nvrhi::BindingSetItem::Sampler(0, sampler));
			bindingSet = Core::NVDevice()->createBindingSet(bindingSetDesc, bindingLayout);

			i = 1;
		}

		for (auto& entity : view)
		{
			const ECS::MeshComponent& meshCmp = view.get<ECS::MeshComponent>(entity);
			const auto& transform = view.get<ECS::TransformComponent>(entity);
			const Mat4 model = transform.GetTransform();
			const auto& mesh = Upload::GetMesh(meshCmp.MeshId);


			const Mat4 mvp = model * camera->GetView() * camera->GetProjection();
			CBuffer cBuffer = { glm::transpose(mvp) , model};

			commandList->writeBuffer(constantBuffer, &cBuffer, sizeof(cBuffer));


			auto graphicsState = nvrhi::GraphicsState()
				.setPipeline(graphicsPipeline)
				.setFramebuffer(frameBuffer)
				.addBindingSet(bindingSet)
				.setDynamicStencilRefValue(0)
				.setViewport(nvrhi::ViewportState().addViewportAndScissorRect(nvrhi::Viewport(surface.Width(), surface.Height())))
				.addVertexBuffer(mesh.VertexBufferBinding)
				.setIndexBuffer(mesh.IndexBufferBinding);

			commandList->setGraphicsState(graphicsState);

			auto drawArguments = nvrhi::DrawArguments()
				.setVertexCount(mesh.IndexCount);

			commandList->drawIndexed(drawArguments);
		}
	}


	void Shutdown()
	{
	}
}
