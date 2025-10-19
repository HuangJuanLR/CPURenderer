#pragma once
#include <memory>

#include "../Texture2D.h"

namespace CPURDR
{
	struct Viewport
	{
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
		float minDepth = 0.0f;
		float maxDepth = 1.0f;
	};

	struct ScissorRect
	{
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
	};

	struct FramebufferAttachments
	{
		std::shared_ptr<Texture2D_RGBA> colorBuffer;
		std::shared_ptr<Texture2D_RFloat> depthBuffer;
	};

	struct ClearValue
	{
		uint32_t color = 0xFF000000;
		float depth = 1.0f;
	};


	class Context
	{
	public:
		Context(int width, int height);
		~Context();

		void CreateFramebuffer(int width, int height);
		void ResizeFramebuffer(int width, int height);
		const FramebufferAttachments& GetFramebuffer() const {return m_Framebuffer;}

		void BeginRenderPass(const ClearValue& clearValue);
		void EndRenderPass();

		void SetViewport(const Viewport& viewport);
		void SetScissor(const ScissorRect& scissor);
		const Viewport& GetViewport() const {return m_Viewport;}
		const ScissorRect& GetScissor() const {return m_Scissor;}

		// Clear
		void ClearColor(uint32_t color);
		void ClearDepth(float depth);
		void Clear(const ClearValue& clearValue);

		bool IsInRenderPass() const {return m_InRenderPass;}
		int GetFramebufferWidth() const {return m_FramebufferWidth;}
		int GetFramebufferHeight() const {return m_FramebufferHeight;}

		Texture2D_RGBA* GetColorBuffer() const {return m_Framebuffer.colorBuffer.get();}
		Texture2D_RFloat* GetDepthBuffer() const {return m_Framebuffer.depthBuffer.get();}

	private:
		FramebufferAttachments m_Framebuffer;
		Viewport m_Viewport;
		ScissorRect m_Scissor;

		int m_FramebufferWidth;
		int m_FramebufferHeight;

		bool m_InRenderPass;
		ClearValue m_ClearValue;
	};
}
