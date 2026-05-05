#include "ReflectionProbe.h"

#include "graphics/RenderTarget.h"


extern SDL_GPUDevice* device;


void InitReflectionProbe(ReflectionProbe* probe, vec3 position, vec3 size)
{
	ColorAttachmentInfo targetInfo = {};
	targetInfo.format = SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT;
	targetInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
	targetInfo.storeOp = SDL_GPU_STOREOP_STORE;
	targetInfo.clearColor = { 0, 0, 0, 0 };
	targetInfo.mips = true;

	DepthAttachmentInfo depthInfo = {};
	depthInfo.format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
	depthInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
	depthInfo.storeOp = SDL_GPU_STOREOP_DONT_CARE;
	depthInfo.clearDepth = 0;
	depthInfo.mips = true;

	probe->renderTarget = CreateRenderTarget(32, 32, SDL_GPU_TEXTURETYPE_CUBE, 1, &targetInfo, &depthInfo);
	probe->position = position;
	probe->size = size;
}
