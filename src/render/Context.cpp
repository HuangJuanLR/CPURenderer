#include "Context.h"

namespace CPURDR
{
	Context::Context(int width, int height):
		m_FramebufferWidth(width), m_FramebufferHeight(height), m_InRenderPass(false)
	{
		CreateFramebuffer(width, height);

		m_Viewport.x = 0;
		m_Viewport.y = 0;
		m_Viewport.width = width;
		m_Viewport.height = height;
		m_Viewport.minDepth = 0.0f;
		m_Viewport.maxDepth = 1.0f;

		m_Scissor.x = 0;
		m_Scissor.y = 0;
		m_Scissor.width = width;
		m_Scissor.height = height;
	}

	Context::~Context()
	{

	}

	void Context::CreateFramebuffer(int width, int height)
	{
		m_FramebufferWidth = width;
		m_FramebufferHeight = height;

		m_Framebuffer.colorBuffer = std::make_shared<Texture2D_RGBA>(width, height, 0x000000FF);
		m_Framebuffer.depthBuffer = std::make_shared<Texture2D_RFloat>(width, height, 0.0f);
	}

	void Context::ResizeFramebuffer(int width, int height)
	{
		if (m_InRenderPass)
		{
			return;
		}

		CreateFramebuffer(width, height);

		m_Viewport.width = width;
		m_Viewport.height = height;
		m_Scissor.width = width;
		m_Scissor.height = height;
	}

	void Context::BeginRenderPass(const ClearValue& clearValue)
	{
		if (m_InRenderPass)
		{
			return;
		}

		m_InRenderPass = true;
		m_ClearValue = clearValue;

		Clear(clearValue);
	}

	void Context::EndRenderPass()
	{
		if (!m_InRenderPass)
		{
			return;
		}
		m_InRenderPass = false;
	}

	void Context::SetViewport(const Viewport& viewport)
	{
		m_Viewport = viewport;

		m_Viewport.x = std::max(0, viewport.x);
		m_Viewport.y = std::max(0, viewport.y);
		m_Viewport.width = std::min(viewport.width, m_FramebufferWidth - m_Viewport.x);
		m_Viewport.height = std::min(viewport.height, m_FramebufferHeight - m_Viewport.y);
	}

	void Context::SetScissor(const ScissorRect& scissor)
	{
		m_Scissor = scissor;

		m_Scissor.x = std::max(0, scissor.x);
		m_Scissor.y = std::max(0, scissor.y);
		m_Scissor.width = std::min(scissor.width, m_FramebufferWidth - m_Scissor.x);
		m_Scissor.height = std::min(scissor.height, m_FramebufferHeight - m_Scissor.y);
	}

	void Context::ClearColor(uint32_t color)
	{
		if (m_Framebuffer.colorBuffer)
		{
			m_Framebuffer.colorBuffer->Clear();
		}
	}


	void Context::ClearDepth(float depth)
	{
		if (m_Framebuffer.depthBuffer)
		{
			m_Framebuffer.depthBuffer->Clear();
		}
	}

	void Context::Clear(const ClearValue& clearValue)
	{
		ClearColor(clearValue.color);
		ClearDepth(clearValue.depth);
	}

}