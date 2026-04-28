#include "RenderTarget.h"

#include "Application.h"


extern SDL_GPUDevice* device;
extern GraphicsState* graphics;


int GetNumMipsForTexture(int width, int height)
{
	int size = max(width, height);
	int numMips = 0;
	while (size)
	{
		numMips++;
		size /= 2;
	}
	return numMips;
}

ivec2 GetMipSize(int width, int height, int mip)
{
	for (int i = 0; i < mip; i++)
	{
		width = max(width / 2, 1);
		height = max(height / 2, 1);
	}
	return ivec2(width, height);
}

static SDL_GPUTexture* CreateColorAttachment(int width, int height, SDL_GPUTextureType textureType, const ColorAttachmentInfo* attachmentInfo)
{
	if (textureType == SDL_GPU_TEXTURETYPE_CUBE)
		SDL_assert(width == height);

	SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.type = textureType;
	textureInfo.format = attachmentInfo->format;
	textureInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | attachmentInfo->usage | (attachmentInfo->storeOp == SDL_GPU_STOREOP_DONT_CARE ? 0 : SDL_GPU_TEXTUREUSAGE_SAMPLER);
	textureInfo.width = width;
	textureInfo.height = height;
	textureInfo.layer_count_or_depth = textureType == SDL_GPU_TEXTURETYPE_CUBE ? 6 : 1;
	textureInfo.num_levels = attachmentInfo->mips ? GetNumMipsForTexture(width, height) : 1;
	textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

	return SDL_CreateGPUTexture(device, &textureInfo);
}

static SDL_GPUTexture* CreateDepthAttachment(int width, int height, SDL_GPUTextureType textureType, const DepthAttachmentInfo* attachmentInfo)
{
	if (textureType == SDL_GPU_TEXTURETYPE_CUBE)
		SDL_assert(width == height);

	SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.type = textureType;
	textureInfo.format = attachmentInfo->format;
	textureInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET | attachmentInfo->usage | (attachmentInfo->storeOp == SDL_GPU_STOREOP_DONT_CARE ? 0 : SDL_GPU_TEXTUREUSAGE_SAMPLER);
	textureInfo.width = width;
	textureInfo.height = height;
	textureInfo.layer_count_or_depth = textureType == SDL_GPU_TEXTURETYPE_CUBE ? 6 : 1;
	textureInfo.num_levels = attachmentInfo->mips ? GetNumMipsForTexture(width, height) : 1;
	textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

	return SDL_CreateGPUTexture(device, &textureInfo);
}

RenderTarget* CreateRenderTarget(int width, int height, SDL_GPUTextureType textureType, int numColorAttachments, const ColorAttachmentInfo* colorAttachmentInfos, const DepthAttachmentInfo* depthAttachmentInfo)
{
	SDL_assert(graphics->numRenderTargets < MAX_RENDER_TARGETS);

	RenderTarget* renderTarget = &graphics->renderTargets[graphics->numRenderTargets++];

	renderTarget->width = width;
	renderTarget->height = height;
	renderTarget->textureType = textureType;

	renderTarget->numColorAttachments = numColorAttachments;
	SDL_memcpy(renderTarget->colorAttachmentInfos, colorAttachmentInfos, numColorAttachments * sizeof(ColorAttachmentInfo));
	for (int i = 0; i < numColorAttachments; i++)
	{
		renderTarget->colorAttachments[i] = CreateColorAttachment(width, height, textureType, &colorAttachmentInfos[i]);
	}

	if (depthAttachmentInfo)
	{
		renderTarget->hasDepthAttachment = true;
		renderTarget->depthAttachmentInfo = *depthAttachmentInfo;
		renderTarget->depthAttachment = CreateDepthAttachment(width, height, textureType, depthAttachmentInfo);
	}

	return renderTarget;
}

void DestroyRenderTarget(RenderTarget* renderTarget)
{
	for (int i = 0; i < renderTarget->numColorAttachments; i++)
	{
		SDL_ReleaseGPUTexture(device, renderTarget->colorAttachments[i]);
	}
	if (renderTarget->hasDepthAttachment)
	{
		SDL_ReleaseGPUTexture(device, renderTarget->depthAttachment);
	}
}

void ResizeRenderTarget(RenderTarget* renderTarget, int width, int height)
{
	for (int i = 0; i < renderTarget->numColorAttachments; i++)
	{
		SDL_ReleaseGPUTexture(device, renderTarget->colorAttachments[i]);
		renderTarget->colorAttachments[i] = CreateColorAttachment(width, height, renderTarget->textureType, &renderTarget->colorAttachmentInfos[i]);
	}
	if (renderTarget->hasDepthAttachment)
	{
		SDL_ReleaseGPUTexture(device, renderTarget->depthAttachment);
		renderTarget->depthAttachment = CreateDepthAttachment(width, height, renderTarget->textureType, &renderTarget->depthAttachmentInfo);
	}

	renderTarget->width = width;
	renderTarget->height = height;
}

SDL_GPURenderPass* BindRenderTarget(RenderTarget* renderTarget, int layer, SDL_GPUCommandBuffer* cmdBuffer)
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
		colorTargetInfo[i].layer_or_depth_plane = layer;
	}

	SDL_GPUDepthStencilTargetInfo depthTargetInfo = {};
	depthTargetInfo.load_op = renderTarget->depthAttachmentInfo.loadOp;
	depthTargetInfo.store_op = renderTarget->depthAttachmentInfo.storeOp;
	depthTargetInfo.clear_depth = renderTarget->depthAttachmentInfo.clearDepth;
	depthTargetInfo.texture = renderTarget->depthAttachment;
	depthTargetInfo.layer = layer;

	return SDL_BeginGPURenderPass(cmdBuffer, colorTargetInfo, renderTarget->numColorAttachments, renderTarget->hasDepthAttachment ? &depthTargetInfo : nullptr);
}
