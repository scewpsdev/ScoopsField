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

	probe->cubemap = CreateRenderTarget(REFLECTION_PROBE_RESOLUTION, REFLECTION_PROBE_RESOLUTION, SDL_GPU_TEXTURETYPE_CUBE, 1, &targetInfo, nullptr);

	SDL_GPUBufferCreateInfo bufferInfo = {};
	bufferInfo.usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE | SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
	bufferInfo.size = 9 * sizeof(vec4);

	probe->irradiance = SDL_CreateGPUBuffer(device, &bufferInfo);

	SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.type = SDL_GPU_TEXTURETYPE_CUBE;
	textureInfo.format = SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT;
	textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE;
	textureInfo.width = 32;
	textureInfo.height = 32;
	textureInfo.layer_count_or_depth = 6;
	textureInfo.num_levels = GetNumMipsForTexture(32, 32);

	probe->specular = SDL_CreateGPUTexture(device, &textureInfo);
}

void DestroyReflectionProbe(ReflectionProbe* probe)
{
	DestroyRenderTarget(probe->cubemap);
	SDL_ReleaseGPUBuffer(device, probe->irradiance);
	SDL_ReleaseGPUTexture(device, probe->specular);
}
