#pragma once



namespace Raito
{
	struct SysWindow;
	namespace Renderer::OpenGL
	{
		enum class FrameBufferTextureFormat
		{
			None = 0,

			// Color
			RGBA8,
			RED_INTEGER,

			// Depth/stencil
			DEPTH24STENCIL8,

			// Defaults
			Depth = DEPTH24STENCIL8
		};

		struct FrameBufferTextureData
		{
			FrameBufferTextureData() = default;
			FrameBufferTextureData(FrameBufferTextureFormat format)
				: TextureFormat(format) {}

			FrameBufferTextureFormat TextureFormat = FrameBufferTextureFormat::None;
		};

		struct FrameBufferAttachmentData
		{
			FrameBufferAttachmentData() = default;
			FrameBufferAttachmentData(std::initializer_list<FrameBufferTextureData> attachments)
				: Attachments(attachments) {}

			std::vector<FrameBufferTextureData> Attachments;
		};

		struct FrameBufferData
		{
			FrameBufferAttachmentData Attachments;
			u32 Width, Height;
			u32 Samples = 1;
			bool SwapChainTarget = false;
		};

		class OpenGLFrameBuffer final
		{
		public:
			OpenGLFrameBuffer(const FrameBufferData& spec);
			~OpenGLFrameBuffer();

			DEFAULT_MOVE_AND_COPY(OpenGLFrameBuffer)

			void Bind() const;
			void UnBind() const;
			void Resize(u32 width, u32 height);

			NODISCARD const FrameBufferData& Data() const { return m_Data; }
			NODISCARD u32 ColorAttachment(u32 index = 0) const { ASSERT(index < m_ColorAttachments.size()); return m_ColorAttachments[index]; }
			void Reset();
		private:

			u32 m_RenderId = 0;
			FrameBufferData m_Data;

			std::vector<u32> m_ColorAttachments;
			std::vector<FrameBufferTextureData> m_ColorAttachmentData;

			u32 m_DepthAttachment = 0;
			FrameBufferTextureData m_DepthBufferData = FrameBufferTextureFormat::None;
		};
	}
}
