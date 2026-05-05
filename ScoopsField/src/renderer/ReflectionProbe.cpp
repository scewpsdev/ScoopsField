#include "ReflectionProbe.h"

#include "graphics/RenderTarget.h"


extern SDL_GPUDevice* device;


void InitReflectionProbe(ReflectionProbe* probe, vec3 position, vec3 size)
{
	probe->position = position;
	probe->size = size;

	ColorAttachmentInfo targetInfo = {};
	targetInfo.format = SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT;
	targetInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
	targetInfo.storeOp = SDL_GPU_STOREOP_STORE;
	targetInfo.clearColor = { 0, 0, 0, 0 };
	targetInfo.mips = true;

	probe->cubemap = CreateRenderTarget(32, 32, SDL_GPU_TEXTURETYPE_CUBE, 1, &targetInfo, nullptr);

	ColorAttachmentInfo irradianceInfo = {};
	irradianceInfo.format = SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT;
	irradianceInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
	irradianceInfo.storeOp = SDL_GPU_STOREOP_STORE;
	irradianceInfo.clearColor = { 0, 0, 0, 0 };

	probe->irradiance = CreateRenderTarget(32, 32, SDL_GPU_TEXTURETYPE_CUBE, 1, &irradianceInfo, nullptr);
}
