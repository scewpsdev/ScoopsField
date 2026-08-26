#pragma once

#include "math/Vector.h"

#include <SDL3/SDL.h>


struct ColorAttachmentInfo
{
	SDL_GPUTextureFormat format;
	SDL_GPULoadOp loadOp;
	SDL_GPUStoreOp storeOp;
	SDL_GPUTextureUsageFlags usage;
	vec4 clearColor;
	bool mips;
};

struct DepthAttachmentInfo
{
	SDL_GPUTextureFormat format;
	SDL_GPUTextureUsageFlags usage;
	bool mips;
	SDL_GPULoadOp loadOp;
	SDL_GPUStoreOp storeOp;
	float clearDepth;
	SDL_GPULoadOp stencilLoadOp;
	SDL_GPUStoreOp stencilStoreOp;
	Uint8 clearStencil;
};

struct RenderTarget
{
	int width, height;
	SDL_GPUTextureType textureType;

#define MAX_COLOR_ATTACHMENTS 8
	SDL_GPUTexture* colorAttachments[MAX_COLOR_ATTACHMENTS];
	ColorAttachmentInfo colorAttachmentInfos[MAX_COLOR_ATTACHMENTS];
	int numColorAttachments;

	bool hasDepthAttachment;
	SDL_GPUTexture* depthAttachment;
	DepthAttachmentInfo depthAttachmentInfo;
};


int GetNumMipsForTexture(int width, int height);
void GetMipSize(int width, int height, int mip, int* outWidth, int* outHeight);

RenderTarget* CreateRenderTarget(int width, int height, SDL_GPUTextureType textureType, int numColorAttachments, const ColorAttachmentInfo* colorAttachmentInfos, const DepthAttachmentInfo* depthAttachmentInfo);
void DestroyRenderTarget(RenderTarget* renderTarget);

void ResizeRenderTarget(RenderTarget* renderTarget, int width, int height);

SDL_GPURenderPass* BindRenderTarget(RenderTarget* renderTarget, int layer, SDL_GPUCommandBuffer* cmdBuffer);
SDL_GPURenderPass* BindRenderTarget(RenderTarget* renderTarget, int layer, SDL_GPULoadOp loadOp, SDL_GPUStoreOp storeOp, SDL_GPUCommandBuffer* cmdBuffer);
