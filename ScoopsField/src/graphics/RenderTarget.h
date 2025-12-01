#pragma once

#include "math/Vector.h"

#include <SDL3/SDL.h>


struct ColorAttachmentInfo
{
	SDL_GPUTextureFormat format;
	SDL_GPULoadOp loadOp;
	SDL_GPUStoreOp storeOp;
	vec4 clearColor;
};

struct DepthAttachmentInfo
{
	SDL_GPUTextureFormat format;
	SDL_GPULoadOp loadOp;
	SDL_GPUStoreOp storeOp;
	float clearDepth;
};

struct RenderTarget
{
	int width, height;

#define MAX_COLOR_ATTACHMENTS 8
	SDL_GPUTexture* colorAttachments[MAX_COLOR_ATTACHMENTS];
	ColorAttachmentInfo colorAttachmentInfos[MAX_COLOR_ATTACHMENTS];
	int numColorAttachments;

	bool hasDepthAttachment;
	SDL_GPUTexture* depthAttachment;
	DepthAttachmentInfo depthAttachmentInfo;
};


RenderTarget* CreateRenderTarget(int width, int height, int numColorAttachments, const ColorAttachmentInfo* colorAttachmentInfos, const DepthAttachmentInfo* depthAttachment);
void DestroyRenderTarget(RenderTarget* renderTarget);

SDL_GPURenderPass* BindRenderTarget(RenderTarget* renderTarget, SDL_GPUCommandBuffer* cmdBuffer);
