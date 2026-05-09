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

	probe->cubemap = CreateRenderTarget(REFLECTION_PROBE_RESOLUTION, REFLECTION_PROBE_RESOLUTION, SDL_GPU_TEXTURETYPE_CUBE, 1, &targetInfo, nullptr);

	SDL_GPUBufferCreateInfo bufferInfo = {};
	bufferInfo.usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE | SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
	bufferInfo.size = 9 * sizeof(vec4);

	probe->irradiance = SDL_CreateGPUBuffer(device, &bufferInfo);
}
