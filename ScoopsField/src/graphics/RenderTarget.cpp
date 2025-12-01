#include "RenderTarget.h"

#include "Application.h"


extern SDL_GPUDevice* device;
extern GraphicsState* graphics;


RenderTarget* CreateRenderTarget(int width, int height, int numColorAttachments, const ColorAttachmentInfo* colorAttachmentInfos, const DepthAttachmentInfo* depthAttachmentInfo)
{
	SDL_assert(graphics->numRenderTargets < MAX_RENDER_TARGETS);

	RenderTarget* renderTarget = &graphics->renderTargets[graphics->numRenderTargets++];

	renderTarget->width = width;
	renderTarget->height = height;
	renderTarget->numColorAttachments = numColorAttachments;

	for (int i = 0; i < numColorAttachments; i++)
	{
		SDL_GPUTextureCreateInfo textureInfo = {};
		textureInfo.type = SDL_GPU_TEXTURETYPE_2D;          /**< The base dimensionality of the texture. */
		textureInfo.format = colorAttachmentInfos[i].format;      /**< The pixel format of the texture. */
		textureInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | (colorAttachmentInfos[i].storeOp == SDL_GPU_STOREOP_DONT_CARE ? 0 : SDL_GPU_TEXTUREUSAGE_SAMPLER);   /**< How the texture is intended to be used by the client. */
		textureInfo.width = width;                     /**< The width of the texture. */
		textureInfo.height = height;                    /**< The height of the texture. */
		textureInfo.layer_count_or_depth = 1;      /**< The layer count or depth of the texture. This value is treated as a layer count on 2D array textures, and as a depth value on 3D textures. */
		textureInfo.num_levels = 1;                /**< The number of mip levels in the texture. */
		textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;  /**< The number of samples per texel. Only applies if the texture is used as a render target. */

		renderTarget->colorAttachments[i] = SDL_CreateGPUTexture(device, &textureInfo);
	}

	SDL_memcpy(renderTarget->colorAttachmentInfos, colorAttachmentInfos, numColorAttachments * sizeof(ColorAttachmentInfo));

	if (depthAttachmentInfo)
	{
		renderTarget->hasDepthAttachment = true;

		SDL_GPUTextureCreateInfo textureInfo = {};
		textureInfo.type = SDL_GPU_TEXTURETYPE_2D;
		textureInfo.format = depthAttachmentInfo->format;
		textureInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET | (depthAttachmentInfo->storeOp == SDL_GPU_STOREOP_DONT_CARE ? 0 : SDL_GPU_TEXTUREUSAGE_SAMPLER);
		textureInfo.width = width;
		textureInfo.height = height;
		textureInfo.layer_count_or_depth = 1;
		textureInfo.num_levels = 1;
		textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

		renderTarget->depthAttachment = SDL_CreateGPUTexture(device, &textureInfo);

		renderTarget->depthAttachmentInfo = *depthAttachmentInfo;
	}

	return renderTarget;
}

void DestroyRenderTarget(RenderTarget* renderTarget)
{
	for (int i = 0; i < renderTarget->numColorAttachments; i++)
	{
		SDL_ReleaseGPUTexture(device, renderTarget->colorAttachments[i]);
	}
}

SDL_GPURenderPass* BindRenderTarget(RenderTarget* renderTarget, SDL_GPUCommandBuffer* cmdBuffer)
{
	SDL_GPUColorTargetInfo colorTargetInfo[MAX_COLOR_ATTACHMENTS];
	for (int i = 0; i < renderTarget->numColorAttachments; i++)
	{
		ColorAttachmentInfo* attachmentInfo = &renderTarget->colorAttachmentInfos[i];

		colorTargetInfo[i] = {};
		colorTargetInfo[i].load_op = attachmentInfo->loadOp;
		colorTargetInfo[i].store_op = attachmentInfo->storeOp;
		colorTargetInfo[i].clear_color = { attachmentInfo->clearColor.r, attachmentInfo->clearColor.g, attachmentInfo->clearColor.b, attachmentInfo->clearColor.a };
		colorTargetInfo[i].texture = renderTarget->colorAttachments[i];
	}

	SDL_GPUDepthStencilTargetInfo depthTargetInfo = {};
	depthTargetInfo.load_op = renderTarget->depthAttachmentInfo.loadOp;
	depthTargetInfo.store_op = renderTarget->depthAttachmentInfo.storeOp;
	depthTargetInfo.clear_depth = renderTarget->depthAttachmentInfo.clearDepth;
	depthTargetInfo.texture = renderTarget->depthAttachment;

	return SDL_BeginGPURenderPass(cmdBuffer, colorTargetInfo, renderTarget->numColorAttachments, renderTarget->hasDepthAttachment ? &depthTargetInfo : nullptr);
}
