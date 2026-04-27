#include "Renderer.h"

#include "Application.h"

#include "graphics/GPUTiming.h"
#include "graphics/GPUVulkan.h"


#include "CloudNoise.cpp"


struct LightInstanceData
{
	vec4 positionRadius;
	vec4 color;
};


static const vec3 cubeVertices[8] = {
	vec3(-1), vec3(1, -1, -1), vec3(-1, -1, 1), vec3(1, -1, 1),
	vec3(-1, 1, -1), vec3(1, 1, -1), vec3(-1, 1, 1), vec3(1, 1, 1),
};
static const uint16_t cubeIndices[36] = {
	0, 2, 6, 6, 4, 0,
	1, 5, 7, 7, 3, 1,
	0, 1, 3, 3, 2, 0,
	4, 6, 7, 7, 5, 4,
	0, 4, 5, 5, 1, 0,
	2, 3, 7, 7, 6, 2,
};


extern AppState* app;
extern GameMemory* memory;
extern SDL_Window* window;
extern SDL_GPUDevice* device;


static SDL_GPUTexture* CreateDepthTarget(int width, int height)
{
	SDL_GPUTextureCreateInfo depthTextureInfo = {};
	depthTextureInfo.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	depthTextureInfo.width = width;
	depthTextureInfo.height = height;
	depthTextureInfo.layer_count_or_depth = 1;
	depthTextureInfo.num_levels = 1;
	depthTextureInfo.type = SDL_GPU_TEXTURETYPE_2D;
	depthTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	return SDL_CreateGPUTexture(device, &depthTextureInfo);
}

static RenderTarget* CreateGBuffer(int width, int height)
{
#define GBUFFER_COLOR_ATTACHMENTS 3
	ColorAttachmentInfo colorAttachments[GBUFFER_COLOR_ATTACHMENTS];
	// normal
	colorAttachments[0] = {};
	colorAttachments[0].format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	colorAttachments[0].loadOp = SDL_GPU_LOADOP_DONT_CARE;
	colorAttachments[0].storeOp = SDL_GPU_STOREOP_STORE;
	// color
	colorAttachments[1] = {};
	colorAttachments[1].format = SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT;
	colorAttachments[1].loadOp = SDL_GPU_LOADOP_DONT_CARE;
	colorAttachments[1].storeOp = SDL_GPU_STOREOP_STORE;
	// material
	colorAttachments[2] = {};
	colorAttachments[2].format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	colorAttachments[2].loadOp = SDL_GPU_LOADOP_DONT_CARE;
	colorAttachments[2].storeOp = SDL_GPU_STOREOP_STORE;

	DepthAttachmentInfo depthAttachment = {};
	depthAttachment.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	depthAttachment.loadOp = SDL_GPU_LOADOP_CLEAR;
	depthAttachment.storeOp = SDL_GPU_STOREOP_STORE;
	depthAttachment.clearDepth = 0;

	return CreateRenderTarget(width, height, SDL_GPU_TEXTURETYPE_2D, GBUFFER_COLOR_ATTACHMENTS, colorAttachments, &depthAttachment);
}

static RenderTarget* CreateHDRTarget(int width, int height)
{
	ColorAttachmentInfo hdrTargetInfo = {};
	hdrTargetInfo.format = SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT;
	hdrTargetInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
	hdrTargetInfo.storeOp = SDL_GPU_STOREOP_STORE;
	hdrTargetInfo.clearColor = { 0, 0, 0, 0 };

	DepthAttachmentInfo hdrDepthInfo = {};
	hdrDepthInfo.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	hdrDepthInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
	hdrDepthInfo.storeOp = SDL_GPU_STOREOP_DONT_CARE;
	hdrDepthInfo.clearDepth = 1;

	return CreateRenderTarget(width, height, SDL_GPU_TEXTURETYPE_2D, 1, &hdrTargetInfo, &hdrDepthInfo);
}

static RenderTarget* CreateHalfResTarget(int width, int height)
{
	ColorAttachmentInfo hdrTargetInfo = {};
	hdrTargetInfo.format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	hdrTargetInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
	hdrTargetInfo.storeOp = SDL_GPU_STOREOP_STORE;
	hdrTargetInfo.clearColor = { 0, 0, 0, 0 };

	return CreateRenderTarget(width / 2, height / 2, SDL_GPU_TEXTURETYPE_2D, 1, &hdrTargetInfo, nullptr);
}

static GraphicsPipeline* CreateGeometryPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->defaultShader, renderer->gbuffer, NUM_MESH_BUFFER_LAYOUTS, renderer->meshLayout);
	pipelineInfo.compareOp = SDL_GPU_COMPAREOP_GREATER;
	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateAnimatedPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->animatedShader, renderer->gbuffer, NUM_ANIMATED_MESH_BUFFER_LAYOUTS, renderer->animatedLayout);
	pipelineInfo.compareOp = SDL_GPU_COMPAREOP_GREATER;
	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateCopyDepthPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->copyDepthShader, renderer->hdrTarget, 1, &renderer->screenQuad.vertexBuffer->layout);

	//pipelineInfo.numColorTargets = 0;
	//pipelineInfo.depthTest = false;
	//pipelineInfo.depthWrite = true;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateDirectionalLightPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->directionalLightShader, renderer->hdrTarget, 1, &renderer->screenQuad.vertexBuffer->layout);

	CreateBlendStateAddPremultiplied(&pipelineInfo.colorTargets[0].blend_state);

	pipelineInfo.depthWrite = false;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreatePointLightPipeline(Renderer* renderer)
{
	VertexBufferLayout bufferLayouts[2] = { renderer->cubeVertexBuffer->layout, renderer->pointLightInstanceBuffer->layout };
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_FRONT, renderer->pointLightShader, renderer->hdrTarget, 2, bufferLayouts);

	CreateBlendStateAddPremultiplied(&pipelineInfo.colorTargets[0].blend_state);

	pipelineInfo.depthWrite = false;
	pipelineInfo.compareOp = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateEnvironmentLightPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->environmentLightShader, renderer->hdrTarget, 1, &renderer->screenQuad.vertexBuffer->layout);

	CreateBlendStateOpaque(&pipelineInfo.colorTargets[0].blend_state);

	pipelineInfo.depthWrite = false;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateSkyPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->skyShader, renderer->skyTarget, 1, &renderer->screenQuad.vertexBuffer->layout);

	//pipelineInfo.compareOp = SDL_GPU_COMPAREOP_GREATER_OR_EQUAL;

	pipelineInfo.depthTest = false;
	pipelineInfo.depthWrite = false;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateSkyUpsamplePipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->skyUpsampleShader, renderer->hdrTarget, 1, &renderer->screenQuad.vertexBuffer->layout);

	//pipelineInfo.compareOp = SDL_GPU_COMPAREOP_GREATER;

	SDL_GPUColorTargetBlendState* blendState = &pipelineInfo.colorTargets[0].blend_state;

	blendState->enable_blend = true;

	blendState->src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
	blendState->dst_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	blendState->color_blend_op = SDL_GPU_BLENDOP_ADD;

	blendState->src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
	blendState->dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
	blendState->alpha_blend_op = SDL_GPU_BLENDOP_ADD;

	pipelineInfo.depthTest = false;
	pipelineInfo.depthWrite = false;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateSkyCubePipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->skyCubeShader, renderer->skyCubemap, 1, &renderer->screenQuad.vertexBuffer->layout);

	//pipelineInfo.compareOp = SDL_GPU_COMPAREOP_GREATER_OR_EQUAL;

	pipelineInfo.depthTest = false;
	pipelineInfo.depthWrite = false;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateTonemappingPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->tonemappingShader, nullptr, 1, &renderer->screenQuad.vertexBuffer->layout);
	return CreateGraphicsPipeline(&pipelineInfo);
}

static SDL_GPUTexture* CreateSkyTransmittanceLUT()
{
	SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.type = SDL_GPU_TEXTURETYPE_2D;
	textureInfo.format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE;
	textureInfo.width = 256;
	textureInfo.height = 64;
	textureInfo.layer_count_or_depth = 1;
	textureInfo.num_levels = 1;
	textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

	return SDL_CreateGPUTexture(device, &textureInfo);
}

static SDL_GPUTexture* CreateSkyMultiScatterLUT()
{
	SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.type = SDL_GPU_TEXTURETYPE_2D;
	textureInfo.format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE;
	textureInfo.width = 32;
	textureInfo.height = 32;
	textureInfo.layer_count_or_depth = 1;
	textureInfo.num_levels = 1;
	textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

	return SDL_CreateGPUTexture(device, &textureInfo);
}

static SDL_GPUTexture* CreateSkyViewLUT()
{
	SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.type = SDL_GPU_TEXTURETYPE_2D;
	textureInfo.format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE;
	textureInfo.width = 256;
	textureInfo.height = 128;
	textureInfo.layer_count_or_depth = 1;
	textureInfo.num_levels = 1;
	textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

	return SDL_CreateGPUTexture(device, &textureInfo);
}

static SDL_GPUTexture* CreateSkyAerialLUT()
{
	SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.type = SDL_GPU_TEXTURETYPE_3D;
	textureInfo.format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE;
	textureInfo.width = 32;
	textureInfo.height = 128;
	textureInfo.layer_count_or_depth = 32;
	textureInfo.num_levels = 1;
	textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

	return SDL_CreateGPUTexture(device, &textureInfo);
}

static SDL_GPUTexture* CreateCloudNoiseTexture(Renderer* renderer, SDL_GPUCommandBuffer* cmdBuffer)
{
	SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.type = SDL_GPU_TEXTURETYPE_3D;
	textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE;
	textureInfo.width = 128;
	textureInfo.height = 128;
	textureInfo.layer_count_or_depth = 128;
	textureInfo.num_levels = 1;
	textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

	SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &textureInfo);

	SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
	bufferBinding.texture = texture;
	bufferBinding.mip_level = 0;
	bufferBinding.layer = 0;
	bufferBinding.cycle = false;

	SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

	SDL_BindGPUComputePipeline(computePass, renderer->cloudNoiseShader->compute);

	SDL_DispatchGPUCompute(computePass, 128 / 8, 128 / 8, 128 / 8);

	SDL_EndGPUComputePass(computePass);

	return texture;
}

static SDL_GPUTexture* CreateCloudNoiseDetailTexture(Renderer* renderer, SDL_GPUCommandBuffer* cmdBuffer)
{
	SDL_GPUTextureCreateInfo textureInfo = {};
	textureInfo.type = SDL_GPU_TEXTURETYPE_3D;
	textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE;
	textureInfo.width = 32;
	textureInfo.height = 32;
	textureInfo.layer_count_or_depth = 32;
	textureInfo.num_levels = 1;
	textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

	SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &textureInfo);

	SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
	bufferBinding.texture = texture;
	bufferBinding.mip_level = 0;
	bufferBinding.layer = 0;
	bufferBinding.cycle = false;

	SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

	SDL_BindGPUComputePipeline(computePass, renderer->cloudNoiseDetailShader->compute);

	SDL_DispatchGPUCompute(computePass, 32 / 8, 32 / 8, 32 / 8);

	SDL_EndGPUComputePass(computePass);

	return texture;
}

void InitRenderer(Renderer* renderer, int width, int height, SDL_GPUCommandBuffer* cmdBuffer)
{
	renderer->width = width;
	renderer->height = height;

	InitList(&renderer->meshes);
	InitList(&renderer->pointLights);

	renderer->depthTexture = CreateDepthTarget(width, height);
	renderer->gbuffer = CreateGBuffer(width, height);
	renderer->hdrTarget = CreateHDRTarget(width, height);
	renderer->skyTarget = CreateHalfResTarget(width, height);
	renderer->skyTarget2 = CreateHalfResTarget(width, height);

	{
		ColorAttachmentInfo hdrTargetInfo = {};
		hdrTargetInfo.format = SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT;
		hdrTargetInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
		hdrTargetInfo.storeOp = SDL_GPU_STOREOP_STORE;
		hdrTargetInfo.clearColor = { 0, 0, 0, 0 };
		hdrTargetInfo.mips = true;

		renderer->skyCubemap = CreateRenderTarget(32, 32, SDL_GPU_TEXTURETYPE_CUBE, 1, &hdrTargetInfo, nullptr);
	}

	// position
	renderer->meshLayout[0].numAttributes = 1;
	renderer->meshLayout[0].attributes[0].location = 0;
	renderer->meshLayout[0].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	// normal
	renderer->meshLayout[1].numAttributes = 1;
	renderer->meshLayout[1].attributes[0].location = 1;
	renderer->meshLayout[1].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	// texcoord
	renderer->meshLayout[2].numAttributes = 1;
	renderer->meshLayout[2].attributes[0].location = 4;
	renderer->meshLayout[2].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;

	// position
	renderer->animatedLayout[0].numAttributes = 1;
	renderer->animatedLayout[0].attributes[0].location = 0;
	renderer->animatedLayout[0].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	// normal
	renderer->animatedLayout[1].numAttributes = 1;
	renderer->animatedLayout[1].attributes[0].location = 1;
	renderer->animatedLayout[1].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	// weights + bone indices
	renderer->animatedLayout[2].numAttributes = 2;
	renderer->animatedLayout[2].attributes[0].location = 2;
	renderer->animatedLayout[2].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	renderer->animatedLayout[2].attributes[1].location = 3;
	renderer->animatedLayout[2].attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	// texcoord
	renderer->animatedLayout[3].numAttributes = 1;
	renderer->animatedLayout[3].attributes[0].location = 4;
	renderer->animatedLayout[3].attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;

	InitScreenQuad(&renderer->screenQuad, cmdBuffer);

	VertexBufferLayout cubeLayout = {};
	cubeLayout.numAttributes = 1;
	cubeLayout.attributes[0].location = 0;
	cubeLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	cubeLayout.perInstance = false;

	renderer->cubeVertexBuffer = CreateVertexBuffer(8, &cubeLayout, 0);
	UpdateVertexBuffer(renderer->cubeVertexBuffer, 0, (const uint8_t*)cubeVertices, sizeof(cubeVertices), cmdBuffer);

	renderer->cubeIndexBuffer = CreateIndexBuffer(36, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	UpdateIndexBuffer(renderer->cubeIndexBuffer, 0, (const uint8_t*)cubeIndices, sizeof(cubeIndices), cmdBuffer);

	VertexBufferLayout pointLightInstanceLayout = {};
	pointLightInstanceLayout.numAttributes = 2;
	pointLightInstanceLayout.attributes[0].location = 1;
	pointLightInstanceLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	pointLightInstanceLayout.attributes[1].location = 2;
	pointLightInstanceLayout.attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	pointLightInstanceLayout.perInstance = true;

	renderer->pointLightInstanceBuffer = CreateVertexBuffer(MAX_POINT_LIGHT_DRAWS, &pointLightInstanceLayout, 0);

	renderer->pointLightInstanceTransferBuffer = CreateTransferBuffer(MAX_POINT_LIGHT_DRAWS * sizeof(LightInstanceData), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);
	MapTransferBuffer(renderer->pointLightInstanceTransferBuffer);

	renderer->defaultShader = LoadGraphicsShader("res/shaders/mesh.vert.bin", "res/shaders/mesh.frag.bin");
	renderer->animatedShader = LoadGraphicsShader("res/shaders/mesh_animated.vert.bin", "res/shaders/mesh.frag.bin");
	renderer->copyDepthShader = LoadGraphicsShader("res/shaders/copy_depth.vert.bin", "res/shaders/copy_depth.frag.bin");
	renderer->directionalLightShader = LoadGraphicsShader("res/shaders/lighting/directional_light.vert.bin", "res/shaders/lighting/directional_light.frag.bin");
	renderer->pointLightShader = LoadGraphicsShader("res/shaders/lighting/point_light.vert.bin", "res/shaders/lighting/point_light.frag.bin");
	renderer->environmentLightShader = LoadGraphicsShader("res/shaders/lighting/environment_light.vert.bin", "res/shaders/lighting/environment_light.frag.bin");
	renderer->skyShader = LoadGraphicsShader("res/shaders/sky/sky.vert.bin", "res/shaders/sky/sky.frag.bin");
	renderer->skyUpsampleShader = LoadGraphicsShader("res/shaders/sky/sky_upsample.vert.bin", "res/shaders/sky/sky_upsample.frag.bin");
	renderer->skyCubeShader = LoadGraphicsShader("res/shaders/sky/sky_cube.vert.bin", "res/shaders/sky/sky_cube.frag.bin");
	renderer->skyTransmittaceLUTShader = LoadComputeShader("res/shaders/sky/transmittance_lut.comp.bin");
	renderer->skyMultiScatterLUTShader = LoadComputeShader("res/shaders/sky/multiscatter_lut.comp.bin");
	renderer->skyViewLUTShader = LoadComputeShader("res/shaders/sky/skyview_lut.comp.bin");
	renderer->skyAerialLUTShader = LoadComputeShader("res/shaders/sky/aerial_lut.comp.bin");
	renderer->cloudNoiseShader = LoadComputeShader("res/shaders/sky/cloud_noise.comp.bin");
	renderer->cloudNoiseDetailShader = LoadComputeShader("res/shaders/sky/cloud_noise_detail.comp.bin");
	renderer->sunColorShader = LoadComputeShader("res/shaders/sky/sun_color.comp.bin");
	renderer->tonemappingShader = LoadGraphicsShader("res/shaders/tonemapping.vert.bin", "res/shaders/tonemapping.frag.bin");

	renderer->geometryPipeline = CreateGeometryPipeline(renderer);
	renderer->animatedPipeline = CreateAnimatedPipeline(renderer);
	renderer->copyDepthPipeline = CreateCopyDepthPipeline(renderer);
	renderer->directionalLightPipeline = CreateDirectionalLightPipeline(renderer);
	renderer->pointLightPipeline = CreatePointLightPipeline(renderer);
	renderer->environmentLightPipeline = CreateEnvironmentLightPipeline(renderer);
	renderer->skyPipeline = CreateSkyPipeline(renderer);
	renderer->skyUpsamplePipeline = CreateSkyUpsamplePipeline(renderer);
	renderer->skyCubePipeline = CreateSkyCubePipeline(renderer);
	renderer->tonemappingPipeline = CreateTonemappingPipeline(renderer);

	SDL_GPUSamplerCreateInfo samplerInfo = {};
	renderer->defaultSampler = SDL_CreateGPUSampler(device, &samplerInfo);

	SDL_GPUSamplerCreateInfo clampedSamplerInfo = {};
	clampedSamplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	clampedSamplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	clampedSamplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	renderer->clampedSampler = SDL_CreateGPUSampler(device, &clampedSamplerInfo);

	SDL_GPUSamplerCreateInfo linearSamplerInfo = {};
	linearSamplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
	linearSamplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
	linearSamplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	linearSamplerInfo.max_lod = VK_LOD_CLAMP_NONE;
	renderer->linearSampler = SDL_CreateGPUSampler(device, &linearSamplerInfo);

	SDL_GPUSamplerCreateInfo linearClampedSamplerInfo = {};
	linearClampedSamplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
	linearClampedSamplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
	linearClampedSamplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	linearClampedSamplerInfo.max_lod = VK_LOD_CLAMP_NONE;
	linearClampedSamplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	linearClampedSamplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	linearClampedSamplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	renderer->linearClampedSampler = SDL_CreateGPUSampler(device, &linearClampedSamplerInfo);

	SDL_GPUSamplerCreateInfo linearClampedVSamplerInfo = {};
	linearClampedVSamplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
	linearClampedVSamplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
	linearClampedVSamplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	linearClampedVSamplerInfo.max_lod = VK_LOD_CLAMP_NONE;
	linearClampedVSamplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	renderer->linearClampedVSampler = SDL_CreateGPUSampler(device, &linearClampedSamplerInfo);

	SDL_GPUBufferCreateInfo emptyBufferInfo = {};
	emptyBufferInfo.size = 4;
	emptyBufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	renderer->emptyBuffer = SDL_CreateGPUBuffer(device, &emptyBufferInfo);

	SDL_GPUTextureCreateInfo emptyTextureInfo = {};
	emptyTextureInfo.type = SDL_GPU_TEXTURETYPE_2D;
	emptyTextureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	emptyTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
	emptyTextureInfo.width = 1;
	emptyTextureInfo.height = 1;
	emptyTextureInfo.layer_count_or_depth = 1;
	emptyTextureInfo.num_levels = 1;
	emptyTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
	renderer->emptyTexture = SDL_CreateGPUTexture(device, &emptyTextureInfo);

	renderer->blueNoise = LoadTexture("res/textures/bluenoise.png.bin", cmdBuffer);

	//renderer->noiseTexture = LoadTexture("res/textures/noise.png.bin", cmdBuffer);
	//renderer->environmentMap = LoadTexture("res/textures/sky/sky_cubemap_equirect.png.bin", cmdBuffer);

	renderer->skyTransmittanceLUT = CreateSkyTransmittanceLUT();
	renderer->skyMultiScatterLUT = CreateSkyMultiScatterLUT();
	renderer->skyViewLUT = CreateSkyViewLUT();
	renderer->skyAerialLUT = CreateSkyAerialLUT();

	renderer->cloudNoise = CreateCloudNoiseTexture(renderer, cmdBuffer);
	renderer->cloudNoiseDetail = CreateCloudNoiseDetailTexture(renderer, cmdBuffer);

	renderer->cloudCoverage = GenerateCloudCoverage(cmdBuffer);
	renderer->cloudLowFrequency = GenerateCloudLowFrequency(cmdBuffer);
	renderer->cloudHighFrequency = GenerateCloudHighFrequency(cmdBuffer);

	SDL_GPUTextureCreateInfo sunColorInfo = {};
	sunColorInfo.type = SDL_GPU_TEXTURETYPE_2D;
	sunColorInfo.format = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;
	sunColorInfo.usage = SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE | SDL_GPU_TEXTUREUSAGE_SAMPLER;
	sunColorInfo.width = 4;
	sunColorInfo.height = 4;
	sunColorInfo.layer_count_or_depth = 1;
	sunColorInfo.num_levels = 1;
	sunColorInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
	renderer->sunColorBuffer = SDL_CreateGPUTexture(device, &sunColorInfo);

	renderer->weather.haziness = 0.01f;
	renderer->weather.cloudCoverage = 0.25f;
	renderer->weather.cloudDensity = 0.0625f;
	renderer->weather.windSpeed = 0.1f;
}

void DestroyRenderer(Renderer* renderer)
{
	SDL_ReleaseGPUSampler(device, renderer->defaultSampler);

	DestroyScreenQuad(&renderer->screenQuad);

	UnmapTransferBuffer(renderer->pointLightInstanceTransferBuffer);
	DestroyTransferBuffer(renderer->pointLightInstanceTransferBuffer);

	DestroyVertexBuffer(renderer->pointLightInstanceBuffer);

	DestroyGraphicsPipeline(renderer->directionalLightPipeline);
	DestroyShader(renderer->directionalLightShader);

	DestroyGraphicsPipeline(renderer->pointLightPipeline);
	DestroyShader(renderer->pointLightShader);

	DestroyGraphicsPipeline(renderer->environmentLightPipeline);
	DestroyShader(renderer->environmentLightShader);

	DestroyRenderTarget(renderer->gbuffer);
	SDL_ReleaseGPUTexture(device, renderer->depthTexture);

	//DestroyTexture(renderer->environmentMap);
}

void ResizeRenderer(Renderer* renderer, int width, int height)
{
	renderer->width = width;
	renderer->height = height;

	if (renderer->depthTexture)
		SDL_ReleaseGPUTexture(device, renderer->depthTexture);
	renderer->depthTexture = CreateDepthTarget(width, height);

	if (renderer->gbuffer)
		DestroyRenderTarget(renderer->gbuffer);
	renderer->gbuffer = CreateGBuffer(width, height);

	if (renderer->hdrTarget)
		DestroyRenderTarget(renderer->hdrTarget);
	renderer->hdrTarget = CreateHDRTarget(width, height);

	if (renderer->skyTarget)
		DestroyRenderTarget(renderer->skyTarget);
	renderer->skyTarget = CreateHalfResTarget(width, height);

	if (renderer->skyTarget2)
		DestroyRenderTarget(renderer->skyTarget2);
	renderer->skyTarget2 = CreateHalfResTarget(width, height);
}

void RenderMesh(Renderer* renderer, Mesh* mesh, Material* material, SkeletonState* skeleton, mat4 transform)
{
	MeshDrawData data = {};
	data.mesh = mesh;
	data.material = material;
	data.skeleton = skeleton;
	data.transform = transform;

	if (skeleton)
		renderer->animatedMeshes.add(data);
	else
		renderer->meshes.add(data);
}

static void RenderModelNode(Renderer* renderer, Model* model, Node* node, AnimationState* animation, mat4 parentTransform)
{
	mat4 nodeTransform = animation ? GetNodeTransform(animation, node) : node->transform;
	mat4 nodeGlobalTransform = parentTransform * nodeTransform;

	for (int i = 0; i < node->numMeshes; i++)
	{
		int meshID = node->meshes[i];
		Mesh* mesh = &model->meshes[meshID];
		Material* material = mesh->materialID != -1 ? &model->materials[mesh->materialID] : nullptr;
		RenderMesh(renderer, mesh, material, animation && mesh->skeletonID != -1 ? &animation->skeletons[mesh->skeletonID] : nullptr, nodeGlobalTransform);
	}

	for (int i = 0; i < node->numChildren; i++)
	{
		RenderModelNode(renderer, model, &model->nodes[node->children[i]], animation, nodeGlobalTransform);
	}
}

void RenderModel(Renderer* renderer, Model* model, AnimationState* animation, mat4 transform)
{
	SDL_assert(model);
	RenderModelNode(renderer, model, &model->nodes[0], animation, transform);
}

void RenderLight(Renderer* renderer, vec3 position, vec3 color)
{
	LightDrawData data = {};
	data.position = position;
	data.color = color;
	renderer->pointLights.add(data);
}

static void SubmitMesh(Renderer* renderer, Mesh* mesh, Material* material, SkeletonState* skeleton, const mat4& transform, const mat4& view, const mat4& pv, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmdBuffer)
{
	if (skeleton)
	{
		SDL_GPUBufferBinding vertexBindings[NUM_ANIMATED_MESH_BUFFER_LAYOUTS] = {};
		vertexBindings[0].buffer = mesh->positionBuffer ? mesh->positionBuffer->buffer : renderer->emptyBuffer;
		vertexBindings[0].offset = 0;
		vertexBindings[1].buffer = mesh->normalBuffer ? mesh->normalBuffer->buffer : renderer->emptyBuffer;
		vertexBindings[1].offset = 0;
		vertexBindings[2].buffer = mesh->weightsBuffer ? mesh->weightsBuffer->buffer : renderer->emptyBuffer;
		vertexBindings[2].offset = 0;
		vertexBindings[3].buffer = mesh->texcoordBuffer ? mesh->texcoordBuffer->buffer : renderer->emptyBuffer;
		vertexBindings[3].offset = 0;

		SDL_BindGPUVertexBuffers(renderPass, 0, vertexBindings, NUM_ANIMATED_MESH_BUFFER_LAYOUTS);
	}
	else
	{
		SDL_GPUBufferBinding vertexBindings[NUM_MESH_BUFFER_LAYOUTS] = {};
		vertexBindings[0].buffer = mesh->positionBuffer ? mesh->positionBuffer->buffer : renderer->emptyBuffer;
		vertexBindings[0].offset = 0;
		vertexBindings[1].buffer = mesh->normalBuffer ? mesh->normalBuffer->buffer : renderer->emptyBuffer;
		vertexBindings[1].offset = 0;
		vertexBindings[2].buffer = mesh->texcoordBuffer ? mesh->texcoordBuffer->buffer : renderer->emptyBuffer;
		vertexBindings[2].offset = 0;

		SDL_BindGPUVertexBuffers(renderPass, 0, vertexBindings, NUM_MESH_BUFFER_LAYOUTS);
	}

	SDL_GPUBufferBinding indexBinding = {};
	indexBinding.buffer = mesh->indexBuffer->buffer;
	indexBinding.offset = 0;

	SDL_BindGPUIndexBuffer(renderPass, &indexBinding, mesh->indexBuffer->elementSize);

	if (skeleton)
	{
		struct UniformData
		{
			mat4 projectionViewModel;
			mat4 viewModel;
			mat4 boneTransforms[MAX_BONES];
		};

		UniformData uniforms = {};
		uniforms.projectionViewModel = pv * transform;
		uniforms.viewModel = view * transform;
		SDL_memcpy(uniforms.boneTransforms, skeleton->boneTransforms, skeleton->numBones * sizeof(mat4));
		SDL_PushGPUVertexUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));
	}
	else
	{
		struct UniformData
		{
			mat4 projectionViewModel;
			mat4 viewModel;
		};

		UniformData uniforms = {};
		uniforms.projectionViewModel = pv * transform;
		uniforms.viewModel = view * transform;
		SDL_PushGPUVertexUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));
	}

	{
		struct UniformData
		{
			vec4 materialData0;
			vec4 materialData1;
		};

		UniformData uniforms = {};
		uniforms.materialData0 = vec4(
			material && material->diffuse ? 1.0f : 0.0f,
			material && material->roughness ? 1.0f : 0.0f,
			material && material->metallic ? 1.0f : 0.0f,
			0);
		uniforms.materialData1 = material ? SRGBToLinear(ARGBToVector(material->color)) : vec4(1);
		SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_GPUTextureSamplerBinding textureBindings[3] = {};
		textureBindings[0].texture = material && material->diffuse ? material->diffuse->handle : renderer->emptyTexture;
		textureBindings[0].sampler = renderer->linearSampler;
		textureBindings[1].texture = material && material->roughness ? material->roughness->handle : renderer->emptyTexture;
		textureBindings[1].sampler = renderer->linearSampler;
		textureBindings[2].texture = material && material->metallic ? material->metallic->handle : renderer->emptyTexture;
		textureBindings[2].sampler = renderer->linearSampler;
		SDL_BindGPUFragmentSamplers(renderPass, 0, textureBindings, 3);
	}

	SDL_DrawGPUIndexedPrimitives(renderPass, mesh->indexCount, 1, 0, 0, 0);
}

static float CalculateLightRadius(vec3 color)
{
	// TODO calculate this based on color and attenuation function
	return 10;
}

// TODO
// render meshes front to back
// frustum culling
// stencil for light volumes
// mesh occlusion culling
// light occlusion culling
// mesh instancing
// proper pbr
// atmospheric scattering

void RendererShow(Renderer* renderer, vec3 cameraPosition, mat4 projection, mat4 view, mat4 pv, vec4 frustumPlanes[6], float near, vec3 sunDirection, SDL_GPUTexture* swapchain, SDL_GPUCommandBuffer* cmdBuffer)
{
	GPU_SCOPE("renderer");

	mat4 pvInv = pv.inverted();
	mat4 projectionInv = projection.inverted();
	mat4 viewInv = view.inverted();

	// transmittance lut
	{
		GPU_SCOPE("transmittance lut");

		SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
		bufferBinding.texture = renderer->skyTransmittanceLUT;
		bufferBinding.mip_level = 0;
		bufferBinding.layer = 0;
		bufferBinding.cycle = false;

		SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

		SDL_BindGPUComputePipeline(computePass, renderer->skyTransmittaceLUTShader->compute);

		SDL_GPUTextureSamplerBinding bindings[2];
		bindings[0].texture = renderer->emptyTexture;
		bindings[0].sampler = renderer->defaultSampler;
		bindings[1].texture = renderer->emptyTexture;
		bindings[1].sampler = renderer->defaultSampler;
		SDL_BindGPUComputeSamplers(computePass, 0, bindings, 2);

		struct UniformData
		{
			vec4 weatherData;
		};

		UniformData uniforms = {};
		uniforms.weatherData = renderer->weather.getData();

		SDL_PushGPUComputeUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_DispatchGPUCompute(computePass, 256 / 32, 64 / 32, 1);

		SDL_EndGPUComputePass(computePass);
	}

	// multiscatter lut
	{
		GPU_SCOPE("multiscatter lut");

		SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
		bufferBinding.texture = renderer->skyMultiScatterLUT;
		bufferBinding.mip_level = 0;
		bufferBinding.layer = 0;
		bufferBinding.cycle = false;

		SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

		SDL_BindGPUComputePipeline(computePass, renderer->skyMultiScatterLUTShader->compute);

		SDL_GPUTextureSamplerBinding bindings[2];
		bindings[0].texture = renderer->skyTransmittanceLUT;
		bindings[0].sampler = renderer->linearClampedSampler;
		bindings[1].texture = renderer->emptyTexture;
		bindings[1].sampler = renderer->defaultSampler;
		SDL_BindGPUComputeSamplers(computePass, 0, bindings, 2);

		struct UniformData
		{
			vec4 weatherData;
		};

		UniformData uniforms = {};
		uniforms.weatherData = renderer->weather.getData();

		SDL_PushGPUComputeUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_DispatchGPUCompute(computePass, 32 / 32, 32 / 32, 1);

		SDL_EndGPUComputePass(computePass);
	}

	// sky view lut
	{
		GPU_SCOPE("sky view lut");

		SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
		bufferBinding.texture = renderer->skyViewLUT;
		bufferBinding.mip_level = 0;
		bufferBinding.layer = 0;
		bufferBinding.cycle = false;

		SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

		SDL_BindGPUComputePipeline(computePass, renderer->skyViewLUTShader->compute);

		SDL_GPUTextureSamplerBinding bindings[3];
		bindings[0].texture = renderer->skyTransmittanceLUT;
		bindings[0].sampler = renderer->linearClampedSampler;
		bindings[1].texture = renderer->skyMultiScatterLUT;
		bindings[1].sampler = renderer->linearClampedSampler;
		bindings[2].texture = renderer->blueNoise->handle;
		bindings[2].sampler = renderer->defaultSampler;
		SDL_BindGPUComputeSamplers(computePass, 0, bindings, 3);

		struct UniformData
		{
			vec4 params;
			vec4 params2;

			vec4 weatherData;
		};
		UniformData uniforms = {};
		uniforms.params = vec4(sunDirection, gameTime);
		uniforms.params2 = vec4(cameraPosition, 0);
		uniforms.weatherData = renderer->weather.getData();

		SDL_PushGPUComputeUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_DispatchGPUCompute(computePass, 256 / 32, 128 / 32, 1);

		SDL_EndGPUComputePass(computePass);
	}

	// cloud noise
	{
		GPU_SCOPE("cloud noise");

		SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
		bufferBinding.texture = renderer->cloudNoise;
		bufferBinding.mip_level = 0;
		bufferBinding.layer = 0;
		bufferBinding.cycle = false;

		SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

		SDL_BindGPUComputePipeline(computePass, renderer->cloudNoiseShader->compute);

		SDL_DispatchGPUCompute(computePass, 128 / 8, 128 / 8, 128 / 8);

		SDL_EndGPUComputePass(computePass);
	}

	// cloud noise detail
	{
		GPU_SCOPE("cloud noise detail");

		SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
		bufferBinding.texture = renderer->cloudNoiseDetail;
		bufferBinding.mip_level = 0;
		bufferBinding.layer = 0;
		bufferBinding.cycle = false;

		SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

		SDL_BindGPUComputePipeline(computePass, renderer->cloudNoiseDetailShader->compute);

		SDL_DispatchGPUCompute(computePass, 32 / 8, 32 / 8, 32 / 8);

		SDL_EndGPUComputePass(computePass);
	}

	// sun color
	{
		GPU_SCOPE("sun color compute");

		SDL_GPUStorageTextureReadWriteBinding bufferBinding = {};
		bufferBinding.texture = renderer->sunColorBuffer;
		bufferBinding.mip_level = 0;
		bufferBinding.layer = 0;
		bufferBinding.cycle = false;

		SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdBuffer, &bufferBinding, 1, nullptr, 0);

		SDL_BindGPUComputePipeline(computePass, renderer->sunColorShader->compute);

		SDL_GPUTextureSamplerBinding bindings[5];
		bindings[0].texture = renderer->cloudLowFrequency->handle;
		bindings[0].sampler = renderer->linearSampler;
		bindings[1].texture = renderer->emptyTexture;
		bindings[1].sampler = renderer->defaultSampler;
		bindings[2].texture = renderer->emptyTexture;
		bindings[2].sampler = renderer->defaultSampler;
		bindings[3].texture = renderer->emptyTexture;
		bindings[3].sampler = renderer->defaultSampler;
		bindings[4].texture = renderer->emptyTexture;
		bindings[4].sampler = renderer->defaultSampler;
		SDL_BindGPUComputeSamplers(computePass, 0, bindings, 5);

		struct UniformData
		{
			vec4 params;
			vec4 params2;

			vec4 weatherData;
		};

		UniformData uniforms = {};
		uniforms.params = vec4(sunDirection, gameTime);
		uniforms.params2 = vec4(cameraPosition, 0);
		uniforms.weatherData = renderer->weather.getData();

		SDL_PushGPUComputeUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_DispatchGPUCompute(computePass, 1, 1, 1);

		SDL_EndGPUComputePass(computePass);
	}

	// geometry pass
	{
		GPU_SCOPE("geometry pass");

		SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->gbuffer, 0, cmdBuffer);

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->geometryPipeline->pipeline);

		for (int i = 0; i < renderer->meshes.size; i++)
		{
			Mesh* mesh = renderer->meshes[i].mesh;
			Material* material = renderer->meshes[i].material;
			const mat4& transform = renderer->meshes[i].transform;
			SubmitMesh(renderer, mesh, material, nullptr, transform, view, pv, renderPass, cmdBuffer);
		}

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->animatedPipeline->pipeline);

		for (int i = 0; i < renderer->animatedMeshes.size; i++)
		{
			Mesh* mesh = renderer->animatedMeshes[i].mesh;
			Material* material = renderer->animatedMeshes[i].material;
			SkeletonState* skeleton = renderer->animatedMeshes[i].skeleton;
			const mat4& transform = renderer->animatedMeshes[i].transform;
			SubmitMesh(renderer, mesh, material, skeleton, transform, view, pv, renderPass, cmdBuffer);
		}

		SDL_EndGPURenderPass(renderPass);
	}

	// sky
	{
		GPU_SCOPE("sky & clouds");

		RenderTarget* rt = app->frameIdx % 2 == 0 ? renderer->skyTarget : renderer->skyTarget2;
		RenderTarget* rt2 = app->frameIdx % 2 == 0 ? renderer->skyTarget2 : renderer->skyTarget;

		SDL_GPURenderPass* renderPass = BindRenderTarget(rt, 0, cmdBuffer);

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->skyPipeline->pipeline);

		struct UniformData
		{
			vec4 params;
			vec4 params2;
			mat4 projectionInv;
			mat4 viewInv;
			mat4 lastProjection;
			mat4 lastView;

			vec4 weatherData;
		};

		UniformData uniforms = {};
		uniforms.params = vec4(sunDirection, gameTime);
		uniforms.params2 = vec4((float)app->frameIdx, 0, 0, 0);
		uniforms.projectionInv = projectionInv;
		uniforms.viewInv = viewInv;
		uniforms.lastProjection = renderer->lastProjection;
		uniforms.lastView = renderer->lastView;
		uniforms.weatherData = renderer->weather.getData();

		SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_GPUTexture* gbufferTextures[11];
		gbufferTextures[0] = renderer->gbuffer->depthAttachment;
		gbufferTextures[1] = renderer->cloudCoverage->handle;
		gbufferTextures[2] = renderer->cloudLowFrequency->handle;
		gbufferTextures[3] = renderer->cloudHighFrequency->handle;
		gbufferTextures[4] = renderer->blueNoise->handle;
		gbufferTextures[5] = rt2->colorAttachments[0];
		gbufferTextures[6] = renderer->skyTransmittanceLUT;
		gbufferTextures[7] = renderer->skyMultiScatterLUT;
		gbufferTextures[8] = renderer->skyViewLUT;
		gbufferTextures[9] = renderer->cloudNoise;
		gbufferTextures[10] = renderer->cloudNoiseDetail;

		SDL_GPUSampler* samplers[11];
		samplers[0] = renderer->defaultSampler;
		samplers[1] = renderer->linearSampler;
		samplers[2] = renderer->linearSampler;
		samplers[3] = renderer->linearSampler;
		samplers[4] = renderer->defaultSampler;
		samplers[5] = renderer->defaultSampler;
		samplers[6] = renderer->linearClampedSampler;
		samplers[7] = renderer->linearClampedSampler;
		samplers[8] = renderer->linearClampedVSampler;
		samplers[9] = renderer->linearSampler;
		samplers[10] = renderer->linearSampler;

		RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 11, gbufferTextures, samplers, cmdBuffer);

		SDL_EndGPURenderPass(renderPass);
	}

	// sky cubemap
	{
		GPU_SCOPE("sky cubemap");

		mat4 cubemapViewsInv[6];
		cubemapViewsInv[SDL_GPU_CUBEMAPFACE_POSITIVEX] = mat4::Rotate(vec3::Up, -0.5f * PI);
		cubemapViewsInv[SDL_GPU_CUBEMAPFACE_NEGATIVEX] = mat4::Rotate(vec3::Up, 0.5f * PI);
		cubemapViewsInv[SDL_GPU_CUBEMAPFACE_POSITIVEY] = mat4::Rotate(vec3::Right, 0.5f * PI);
		cubemapViewsInv[SDL_GPU_CUBEMAPFACE_NEGATIVEY] = mat4::Rotate(vec3::Right, -0.5f * PI);
		cubemapViewsInv[SDL_GPU_CUBEMAPFACE_POSITIVEZ] = mat4::Rotate(vec3::Up, PI);
		cubemapViewsInv[SDL_GPU_CUBEMAPFACE_NEGATIVEZ] = mat4::Identity;

		for (int i = 0; i < 6; i++)
		{
			SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->skyCubemap, i, cmdBuffer);

			SDL_BindGPUGraphicsPipeline(renderPass, renderer->skyCubePipeline->pipeline);

			struct UniformData
			{
				vec4 params;
				vec4 params2;
				mat4 projectionInv;
				mat4 viewInv;

				vec4 weatherData;
			};

			UniformData uniforms = {};
			uniforms.params = vec4(sunDirection, gameTime);
			uniforms.params2 = vec4(cameraPosition, (float)app->frameIdx);
			uniforms.projectionInv = mat4::Perspective(0.5f * PI, 1, 0.1f);
			uniforms.viewInv = cubemapViewsInv[i];
			uniforms.weatherData = renderer->weather.getData();

			SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

			SDL_GPUTexture* gbufferTextures[9];
			gbufferTextures[0] = renderer->cloudCoverage->handle;
			gbufferTextures[1] = renderer->cloudLowFrequency->handle;
			gbufferTextures[2] = renderer->cloudHighFrequency->handle;
			gbufferTextures[3] = renderer->blueNoise->handle;
			gbufferTextures[4] = renderer->skyTransmittanceLUT;
			gbufferTextures[5] = renderer->skyMultiScatterLUT;
			gbufferTextures[6] = renderer->skyViewLUT;
			gbufferTextures[7] = renderer->cloudNoise;
			gbufferTextures[8] = renderer->cloudNoiseDetail;

			SDL_GPUSampler* samplers[9];
			samplers[0] = renderer->linearSampler;
			samplers[1] = renderer->linearSampler;
			samplers[2] = renderer->linearSampler;
			samplers[3] = renderer->defaultSampler;
			samplers[4] = renderer->linearClampedSampler;
			samplers[5] = renderer->linearClampedSampler;
			samplers[6] = renderer->linearClampedVSampler;
			samplers[7] = renderer->linearSampler;
			samplers[8] = renderer->linearSampler;

			RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 9, gbufferTextures, samplers, cmdBuffer);

			SDL_EndGPURenderPass(renderPass);
		}

		SDL_GenerateMipmapsForGPUTexture(cmdBuffer, renderer->skyCubemap->colorAttachments[0]);
	}

	// lighting pass
	{
		GPU_SCOPE("lighting");

		{
			GPU_SCOPE("copy pass");

			// update point light data
			SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);
			LightInstanceData* instanceData = (LightInstanceData*)renderer->pointLightInstanceTransferBuffer->mapped;
			for (int i = 0; i < renderer->pointLights.size; i++)
			{
				vec3 position = renderer->pointLights[i].position;
				vec3 color = renderer->pointLights[i].color;
				instanceData[i].positionRadius = vec4(position, CalculateLightRadius(color));
				instanceData[i].color = vec4(color, 0);
			}
			UpdateVertexBuffer(renderer->pointLightInstanceBuffer, 0, renderer->pointLights.size * sizeof(LightInstanceData), renderer->pointLightInstanceTransferBuffer->buffer, copyPass);
			SDL_EndGPUCopyPass(copyPass); copyPass = nullptr;
		}

		//GPU_SCOPE("render pass");

		SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->hdrTarget, 0, cmdBuffer);

		// copy depth
		{
			GPU_SCOPE("copy depth");

			SDL_BindGPUGraphicsPipeline(renderPass, renderer->copyDepthPipeline->pipeline);

			RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 1, &renderer->gbuffer->depthAttachment, &renderer->defaultSampler, cmdBuffer);
		}

		// environment light
		{
			GPU_SCOPE("environment");

			SDL_BindGPUGraphicsPipeline(renderPass, renderer->environmentLightPipeline->pipeline);

			float environmentIntensity = 1.0f;

			struct UniformData
			{
				vec4 params;
				mat4 projectionViewInv;
				mat4 viewInv;
			};
			UniformData uniforms = {};
			uniforms.params = vec4(cameraPosition, environmentIntensity);
			uniforms.projectionViewInv = pvInv;
			uniforms.viewInv = viewInv;
			SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

			SDL_GPUTexture* gbufferTextures[MAX_COLOR_ATTACHMENTS + 2];
			for (int i = 0; i < renderer->gbuffer->numColorAttachments; i++)
				gbufferTextures[i] = renderer->gbuffer->colorAttachments[i];
			gbufferTextures[renderer->gbuffer->numColorAttachments] = renderer->gbuffer->depthAttachment;
			gbufferTextures[renderer->gbuffer->numColorAttachments + 1] = renderer->skyCubemap->colorAttachments[0];

			SDL_GPUSampler* samplers[MAX_COLOR_ATTACHMENTS + 2];
			for (int i = 0; i < MAX_COLOR_ATTACHMENTS + 2; i++)
				samplers[i] = renderer->defaultSampler;
			samplers[renderer->gbuffer->numColorAttachments + 1] = renderer->linearSampler;

			RenderScreenQuad(&renderer->screenQuad, 1, renderPass, renderer->gbuffer->numColorAttachments + 2, gbufferTextures, samplers, cmdBuffer);
		}

		// directional lights
		{
			GPU_SCOPE("sun");

			SDL_BindGPUGraphicsPipeline(renderPass, renderer->directionalLightPipeline->pipeline);

			vec3 lightDirection = (view * vec4(sunDirection, 0)).xyz;

			vec3 lightColor = vec3(1, 1, 1);
			float lightIntensity = 10;
			lightColor *= lightIntensity;

			struct UniformData
			{
				vec4 lightDirection;
				vec4 lightColor;
				mat4 projection;
			};
			UniformData uniforms = {};
			uniforms.lightDirection = vec4(lightDirection, gameTime);
			uniforms.lightColor = vec4(lightColor, 0);
			uniforms.projection = projection;
			SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

			SDL_GPUTexture* gbufferTextures[MAX_COLOR_ATTACHMENTS + 2];
			for (int i = 0; i < renderer->gbuffer->numColorAttachments; i++)
				gbufferTextures[i] = renderer->gbuffer->colorAttachments[i];
			gbufferTextures[renderer->gbuffer->numColorAttachments] = renderer->gbuffer->depthAttachment;
			gbufferTextures[renderer->gbuffer->numColorAttachments + 1] = renderer->sunColorBuffer;

			SDL_GPUSampler* samplers[MAX_COLOR_ATTACHMENTS + 2];
			for (int i = 0; i < MAX_COLOR_ATTACHMENTS + 2; i++)
				samplers[i] = renderer->defaultSampler;
			samplers[renderer->gbuffer->numColorAttachments + 1] = renderer->defaultSampler;

			RenderScreenQuad(&renderer->screenQuad, 1, renderPass, renderer->gbuffer->numColorAttachments + 2, gbufferTextures, samplers, cmdBuffer);
		}

		// point lights
		{
			GPU_SCOPE("point lights");

			SDL_BindGPUGraphicsPipeline(renderPass, renderer->pointLightPipeline->pipeline);

			SDL_GPUBufferBinding vertexBindings[2];
			vertexBindings[0] = {};
			vertexBindings[0].buffer = renderer->cubeVertexBuffer->buffer;
			vertexBindings[0].offset = 0;
			vertexBindings[1] = {};
			vertexBindings[1].buffer = renderer->pointLightInstanceBuffer->buffer;
			vertexBindings[1].offset = 0;

			SDL_BindGPUVertexBuffers(renderPass, 0, vertexBindings, 2);

			SDL_GPUBufferBinding indexBinding = {};
			indexBinding.buffer = renderer->cubeIndexBuffer->buffer;
			indexBinding.offset = 0;

			SDL_BindGPUIndexBuffer(renderPass, &indexBinding, renderer->cubeIndexBuffer->elementSize);

			struct VertexUniformData
			{
				mat4 pv;
				mat4 view;
			};
			VertexUniformData vertexUniforms = {};
			vertexUniforms.pv = pv;
			vertexUniforms.view = view;
			SDL_PushGPUVertexUniformData(cmdBuffer, 0, &vertexUniforms, sizeof(vertexUniforms));

			struct UniformData
			{
				mat4 projection;
				vec4 viewTexel;
			};
			UniformData uniforms = {};
			uniforms.projection = projection;
			uniforms.viewTexel = vec4(1.0f / renderer->hdrTarget->width, 1.0f / renderer->hdrTarget->height, 0, 0);
			SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

			SDL_GPUTextureSamplerBinding textureBindings[MAX_COLOR_ATTACHMENTS + 1];
			for (int i = 0; i < renderer->gbuffer->numColorAttachments; i++)
			{
				textureBindings[i].texture = renderer->gbuffer->colorAttachments[i];
				textureBindings[i].sampler = renderer->defaultSampler;
			}
			textureBindings[renderer->gbuffer->numColorAttachments].texture = renderer->gbuffer->depthAttachment;
			textureBindings[renderer->gbuffer->numColorAttachments].sampler = renderer->defaultSampler;

			SDL_BindGPUFragmentSamplers(renderPass, 0, textureBindings, renderer->gbuffer->numColorAttachments + 1);

			SDL_DrawGPUIndexedPrimitives(renderPass, renderer->cubeIndexBuffer->numIndices, renderer->pointLights.size, 0, 0, 0);
		}

		// sky upsample
		{
			GPU_SCOPE("sky upsample");

			SDL_BindGPUGraphicsPipeline(renderPass, renderer->skyUpsamplePipeline->pipeline);

			RenderTarget* rt = app->frameIdx % 2 == 0 ? renderer->skyTarget : renderer->skyTarget2;

			SDL_GPUTexture* textures[3];
			textures[0] = rt->colorAttachments[0];
			textures[1] = renderer->gbuffer->depthAttachment;

			SDL_GPUSampler* samplers[3];
			samplers[0] = renderer->linearSampler;
			samplers[1] = renderer->clampedSampler;

			RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 2, textures, samplers, cmdBuffer);
		}

		SDL_EndGPURenderPass(renderPass);
	}

	// tonemapping
	{
		GPU_SCOPE("tonemap");

		SDL_GPUColorTargetInfo colorTarget = {};
		colorTarget.load_op = SDL_GPU_LOADOP_DONT_CARE;
		colorTarget.store_op = SDL_GPU_STOREOP_STORE;
		colorTarget.texture = swapchain;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdBuffer, &colorTarget, 1, nullptr);

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->tonemappingPipeline->pipeline);

		RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 1, &renderer->hdrTarget->colorAttachments[0], &renderer->defaultSampler, cmdBuffer);

		SDL_EndGPURenderPass(renderPass);
	}

	renderer->lastProjection = projection;
	renderer->lastView = view;

	renderer->meshes.clear();
	renderer->animatedMeshes.clear();
	renderer->pointLights.clear();
}
