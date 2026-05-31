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
#define GBUFFER_COLOR_ATTACHMENTS 4
	ColorAttachmentInfo colorAttachments[GBUFFER_COLOR_ATTACHMENTS];
	// normal
	colorAttachments[0] = {};
	colorAttachments[0].format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	colorAttachments[0].loadOp = SDL_GPU_LOADOP_DONT_CARE;
	colorAttachments[0].storeOp = SDL_GPU_STOREOP_STORE;
	// color
	colorAttachments[1] = {};
	colorAttachments[1].format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	colorAttachments[1].loadOp = SDL_GPU_LOADOP_DONT_CARE;
	colorAttachments[1].storeOp = SDL_GPU_STOREOP_STORE;
	// material
	colorAttachments[2] = {};
	colorAttachments[2].format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	colorAttachments[2].loadOp = SDL_GPU_LOADOP_DONT_CARE;
	colorAttachments[2].storeOp = SDL_GPU_STOREOP_STORE;
	// emissive
	colorAttachments[3] = {};
	colorAttachments[3].format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	colorAttachments[3].loadOp = SDL_GPU_LOADOP_DONT_CARE;
	colorAttachments[3].storeOp = SDL_GPU_STOREOP_STORE;

	DepthAttachmentInfo depthAttachment = {};
	depthAttachment.format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
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
	hdrTargetInfo.mips = true;

	DepthAttachmentInfo hdrDepthInfo = {};
	hdrDepthInfo.format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
	hdrDepthInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
	hdrDepthInfo.storeOp = SDL_GPU_STOREOP_STORE;
	hdrDepthInfo.clearDepth = 1;

	return CreateRenderTarget(width, height, SDL_GPU_TEXTURETYPE_2D, 1, &hdrTargetInfo, &hdrDepthInfo);
}

static RenderTarget* CreateSkyTarget(int width, int height)
{
	ColorAttachmentInfo hdrTargetInfo = {};
	hdrTargetInfo.format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	hdrTargetInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
	hdrTargetInfo.storeOp = SDL_GPU_STOREOP_STORE;
	hdrTargetInfo.clearColor = { 0, 0, 0, 0 };

	return CreateRenderTarget(width, height, SDL_GPU_TEXTURETYPE_2D, 1, &hdrTargetInfo, nullptr);
}

static RenderTarget* CreateShadowMap(int resolution)
{
	DepthAttachmentInfo depthInfo = {};
	depthInfo.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
	depthInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
	depthInfo.storeOp = SDL_GPU_STOREOP_STORE;
	depthInfo.clearDepth = 1;

	return CreateRenderTarget(resolution, resolution, SDL_GPU_TEXTURETYPE_2D, 0, nullptr, &depthInfo);
}

static RenderTarget* CreateShadowBuffer(int width, int height)
{
	ColorAttachmentInfo targetInfo = {};
	targetInfo.format = SDL_GPU_TEXTUREFORMAT_R8_UNORM;
	targetInfo.loadOp = SDL_GPU_LOADOP_DONT_CARE;
	targetInfo.storeOp = SDL_GPU_STOREOP_STORE;

	return CreateRenderTarget(width, height, SDL_GPU_TEXTURETYPE_2D, 1, &targetInfo, nullptr);
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

static GraphicsPipeline* CreateShadowMapPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->depthShader, renderer->shadowMaps[0], NUM_MESH_BUFFER_LAYOUTS, renderer->meshLayout);
	pipelineInfo.compareOp = SDL_GPU_COMPAREOP_LESS;
	pipelineInfo.depthClamp = true;
	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateAnimatedShadowMapPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->animatedDepthShader, renderer->shadowMaps[0], NUM_ANIMATED_MESH_BUFFER_LAYOUTS, renderer->animatedLayout);
	pipelineInfo.compareOp = SDL_GPU_COMPAREOP_LESS;
	pipelineInfo.depthClamp = true;
	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateShadowPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->shadowShader, renderer->shadowBuffer0, 1, &renderer->screenQuad.vertexBuffer->layout);

	pipelineInfo.depthWrite = false;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateBlurHPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->blurHShader, renderer->shadowBuffer1, 1, &renderer->screenQuad.vertexBuffer->layout);

	pipelineInfo.depthWrite = false;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateBlurVPipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->blurVShader, renderer->shadowBuffer1, 1, &renderer->screenQuad.vertexBuffer->layout);

	pipelineInfo.depthWrite = false;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateCopyDepthPipeline(Renderer* renderer, RenderTarget* target)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->copyDepthShader, target, 1, &renderer->screenQuad.vertexBuffer->layout);

	//pipelineInfo.depthTest = false;
	pipelineInfo.compareOp = SDL_GPU_COMPAREOP_ALWAYS;

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

static GraphicsPipeline* CreateReflectionProbePipeline(Renderer* renderer)
{
	VertexBufferLayout bufferLayouts[1] = { renderer->cubeVertexBuffer->layout };
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->reflectionProbeShader, renderer->hdrTarget, 1, bufferLayouts);

	CreateBlendStateAlpha(&pipelineInfo.colorTargets[0].blend_state);

	pipelineInfo.depthWrite = false;
	pipelineInfo.depthClamp = true;
	pipelineInfo.compareOp = SDL_GPU_COMPAREOP_GREATER;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateDeferredDiffusePipeline(Renderer* renderer)
{
	SDL_GPUTextureFormat colorAttachmentFormat = SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT;

	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->deferredDiffuseShader,
		1, &colorAttachmentFormat, false, SDL_GPU_TEXTUREFORMAT_INVALID, 1, &renderer->screenQuad.vertexBuffer->layout);

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

	pipelineInfo.compareOp = SDL_GPU_COMPAREOP_GREATER_OR_EQUAL;

	SDL_GPUColorTargetBlendState* blendState = &pipelineInfo.colorTargets[0].blend_state;

	blendState->enable_blend = true;

	blendState->src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
	blendState->dst_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	blendState->color_blend_op = SDL_GPU_BLENDOP_ADD;

	blendState->src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
	blendState->dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
	blendState->alpha_blend_op = SDL_GPU_BLENDOP_ADD;

	pipelineInfo.depthTest = true;
	pipelineInfo.depthWrite = false;

	return CreateGraphicsPipeline(&pipelineInfo);
}

static GraphicsPipeline* CreateSkyCubePipeline(Renderer* renderer)
{
	GraphicsPipelineInfo pipelineInfo = CreateGraphicsPipelineInfo(SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, SDL_GPU_CULLMODE_BACK, renderer->skyCubeShader, renderer->skyCubemap, 1, &renderer->screenQuad.vertexBuffer->layout);

	CreateBlendStateAlpha(&pipelineInfo.colorTargets[0].blend_state);

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

static void SubmitMesh(Renderer* renderer, VertexBuffer* vertexBuffers[], int numVertexBuffers, IndexBuffer* indexBuffer, int vertexCount, int indexCount, Material* material, SkeletonState* skeleton, const mat4& transform, const mat4& view, const mat4& pv, vec3 cameraPosition, bool viewSpaceBuffer, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmdBuffer);

#define SHADOW_MAP_RESOLUTION 1024

#include "Sky.cpp"
#include "AutoExposure.cpp"
#include "Lighting.cpp"
#include "ShadowMapping.cpp"
#include "ReflectionProbeUpdate.cpp"

void InitRenderer(Renderer* renderer, int width, int height, SDL_GPUCommandBuffer* cmdBuffer)
{
	renderer->width = width;
	renderer->height = height;

	InitList(&renderer->meshes);
	InitList(&renderer->animatedMeshes);
	InitList(&renderer->forwardMeshes);
	InitList(&renderer->pointLights);

	renderer->depthTexture = CreateDepthTarget(width, height);
	renderer->gbuffer = CreateGBuffer(width, height);
	renderer->hdrTarget = CreateHDRTarget(width, height);
	renderer->skyTarget = CreateSkyTarget(width / 2, height / 2);
	renderer->skyTarget2 = CreateSkyTarget(width / 2, height / 2);
	renderer->shadowMaps[0] = CreateShadowMap(SHADOW_MAP_RESOLUTION);
	renderer->shadowMaps[1] = CreateShadowMap(SHADOW_MAP_RESOLUTION * 2);
	renderer->shadowMaps[2] = CreateShadowMap(SHADOW_MAP_RESOLUTION * 2);
	renderer->shadowBuffer0 = CreateShadowBuffer(width / 2, height / 2);
	renderer->shadowBuffer1 = CreateShadowBuffer(width / 2, height / 2);
	for (int i = 0; i < 6; i++)
		renderer->cubemapGbuffers[i] = CreateGBuffer(REFLECTION_PROBE_RESOLUTION, REFLECTION_PROBE_RESOLUTION);
	renderer->reflectionProbeShadowMap = CreateShadowMap(256);

	{
		ColorAttachmentInfo hdrTargetInfo = {};
		hdrTargetInfo.format = SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT;
		hdrTargetInfo.loadOp = SDL_GPU_LOADOP_LOAD;
		hdrTargetInfo.storeOp = SDL_GPU_STOREOP_STORE;
		//hdrTargetInfo.clearColor = { 0, 0, 0, 0 };
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
	renderer->depthShader = LoadGraphicsShader("res/shaders/mesh.vert.bin", "res/shaders/mesh_depth.frag.bin");
	renderer->animatedDepthShader = LoadGraphicsShader("res/shaders/mesh_animated.vert.bin", "res/shaders/mesh_depth.frag.bin");
	renderer->shadowShader = LoadGraphicsShader("res/shaders/screenquad.vert.bin", "res/shaders/lighting/shadow.frag.bin");
	renderer->blurHShader = LoadGraphicsShader("res/shaders/screenquad.vert.bin", "res/shaders/blurh.frag.bin");
	renderer->blurVShader = LoadGraphicsShader("res/shaders/screenquad.vert.bin", "res/shaders/blurv.frag.bin");
	renderer->copyDepthShader = LoadGraphicsShader("res/shaders/screenquad.vert.bin", "res/shaders/copy_depth.frag.bin");
	renderer->directionalLightShader = LoadGraphicsShader("res/shaders/screenquad.vert.bin", "res/shaders/lighting/directional_light.frag.bin");
	renderer->pointLightShader = LoadGraphicsShader("res/shaders/lighting/point_light.vert.bin", "res/shaders/lighting/point_light.frag.bin");
	renderer->environmentLightShader = LoadGraphicsShader("res/shaders/screenquad.vert.bin", "res/shaders/lighting/environment_light.frag.bin");
	renderer->reflectionProbeShader = LoadGraphicsShader("res/shaders/lighting/reflection_probe.vert.bin", "res/shaders/lighting/reflection_probe.frag.bin");
	renderer->deferredDiffuseShader = LoadGraphicsShader("res/shaders/screenquad.vert.bin", "res/shaders/lighting/deferred_diffuse.frag.bin");
	renderer->shConvoluteShader = LoadComputeShader("res/shaders/lighting/sh_convolute.comp.bin");
	renderer->skyShader = LoadGraphicsShader("res/shaders/screenquad.vert.bin", "res/shaders/sky/sky.frag.bin");
	renderer->skyUpsampleShader = LoadGraphicsShader("res/shaders/screenquad.vert.bin", "res/shaders/sky/sky_upsample.frag.bin");
	renderer->skyCubeShader = LoadGraphicsShader("res/shaders/screenquad.vert.bin", "res/shaders/sky/sky_cube.frag.bin");
	renderer->skyTransmittanceLUTShader = LoadComputeShader("res/shaders/sky/transmittance_lut.comp.bin");
	renderer->skyMultiScatterLUTShader = LoadComputeShader("res/shaders/sky/multiscatter_lut.comp.bin");
	renderer->skyViewLUTShader = LoadComputeShader("res/shaders/sky/skyview_lut.comp.bin");
	renderer->skyAerialLUTShader = LoadComputeShader("res/shaders/sky/aerial_lut.comp.bin");
	renderer->cloudNoiseShader = LoadComputeShader("res/shaders/sky/cloud_noise.comp.bin");
	renderer->cloudNoiseDetailShader = LoadComputeShader("res/shaders/sky/cloud_noise_detail.comp.bin");
	renderer->sunColorShader = LoadComputeShader("res/shaders/sky/sun_color.comp.bin");
	renderer->tonemappingShader = LoadGraphicsShader("res/shaders/screenquad.vert.bin", "res/shaders/tonemapping.frag.bin");

	renderer->geometryPipeline = CreateGeometryPipeline(renderer);
	renderer->animatedPipeline = CreateAnimatedPipeline(renderer);
	renderer->shadowMapPipeline = CreateShadowMapPipeline(renderer);
	renderer->animatedShadowMapPipeline = CreateAnimatedShadowMapPipeline(renderer);
	renderer->shadowPipeline = CreateShadowPipeline(renderer);
	renderer->blurHPipeline = CreateBlurHPipeline(renderer);
	renderer->blurVPipeline = CreateBlurVPipeline(renderer);
	renderer->copyDepthPipeline = CreateCopyDepthPipeline(renderer, renderer->hdrTarget);
	renderer->copyDepthPipeline2 = CreateCopyDepthPipeline(renderer, renderer->gbuffer);
	renderer->directionalLightPipeline = CreateDirectionalLightPipeline(renderer);
	renderer->pointLightPipeline = CreatePointLightPipeline(renderer);
	renderer->environmentLightPipeline = CreateEnvironmentLightPipeline(renderer);
	renderer->reflectionProbePipeline = CreateReflectionProbePipeline(renderer);
	renderer->deferredDiffusePipeline = CreateDeferredDiffusePipeline(renderer);
	renderer->skyPipeline = CreateSkyPipeline(renderer);
	renderer->skyUpsamplePipeline = CreateSkyUpsamplePipeline(renderer);
	renderer->skyCubePipeline = CreateSkyCubePipeline(renderer);
	renderer->tonemappingPipeline = CreateTonemappingPipeline(renderer);

	SDL_GPUSamplerCreateInfo samplerInfo = {};
	renderer->samplers[TEXTURE_SAMPLER_DEFAULT] = SDL_CreateGPUSampler(device, &samplerInfo);

	SDL_GPUSamplerCreateInfo clampedSamplerInfo = {};
	clampedSamplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	clampedSamplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	clampedSamplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	renderer->samplers[TEXTURE_SAMPLER_CLAMPED] = SDL_CreateGPUSampler(device, &clampedSamplerInfo);

	SDL_GPUSamplerCreateInfo linearSamplerInfo = {};
	linearSamplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
	linearSamplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
	linearSamplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	linearSamplerInfo.max_lod = VK_LOD_CLAMP_NONE;
	renderer->samplers[TEXTURE_SAMPLER_LINEAR] = SDL_CreateGPUSampler(device, &linearSamplerInfo);

	SDL_GPUSamplerCreateInfo linearClampedSamplerInfo = {};
	linearClampedSamplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
	linearClampedSamplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
	linearClampedSamplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	linearClampedSamplerInfo.max_lod = VK_LOD_CLAMP_NONE;
	linearClampedSamplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	linearClampedSamplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	linearClampedSamplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	renderer->samplers[TEXTURE_SAMPLER_LINEAR_CLAMPED] = SDL_CreateGPUSampler(device, &linearClampedSamplerInfo);

	SDL_GPUSamplerCreateInfo linearClampedVSamplerInfo = {};
	linearClampedVSamplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
	linearClampedVSamplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
	linearClampedVSamplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	linearClampedVSamplerInfo.max_lod = VK_LOD_CLAMP_NONE;
	linearClampedVSamplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	renderer->samplers[TEXTURE_SAMPLER_LINEAR_CLAMPED_VERTICAL] = SDL_CreateGPUSampler(device, &linearClampedSamplerInfo);

	SDL_GPUSamplerCreateInfo shadowSamplerInfo = {};
	shadowSamplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
	shadowSamplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
	shadowSamplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	shadowSamplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	shadowSamplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	shadowSamplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	shadowSamplerInfo.enable_compare = true;
	shadowSamplerInfo.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
	renderer->samplers[TEXTURE_SAMPLER_SHADOW_LINEAR_CLAMPED] = SDL_CreateGPUSampler(device, &shadowSamplerInfo);

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

	SDL_GPUTransferBufferCreateInfo luminanceReadbackBufferInfo = {};
	luminanceReadbackBufferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
	luminanceReadbackBufferInfo.size = 4 * 4;
	renderer->luminanceReadbackBuffer = SDL_CreateGPUTransferBuffer(device, &luminanceReadbackBufferInfo);
	renderer->currentExposure = 1.0f;
	renderer->targetExposure = 1.0f;

	renderer->blueNoise = LoadTexture("res/textures/bluenoise.png.bin", cmdBuffer);

	//renderer->noiseTexture = LoadTexture("res/textures/noise.png.bin", cmdBuffer);
	//renderer->environmentMap = LoadTexture("res/textures/sky/sky_cubemap_equirect.png.bin", cmdBuffer);

	renderer->skyTransmittanceLUT = CreateSkyTransmittanceLUT();
	renderer->skyMultiScatterLUT = CreateSkyMultiScatterLUT();
	renderer->skyViewLUT = CreateSkyViewLUT();
	renderer->skyAerialLUT = CreateSkyAerialLUT();

	renderer->cloudNoise = CreateCloudNoiseTexture(renderer, cmdBuffer);
	renderer->cloudNoiseDetail = CreateCloudNoiseDetailTexture(renderer, cmdBuffer);

	SDL_GPUTextureCreateInfo sunColorInfo = {};
	sunColorInfo.type = SDL_GPU_TEXTURETYPE_2D;
	sunColorInfo.format = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;
	sunColorInfo.usage = SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_SIMULTANEOUS_READ_WRITE | SDL_GPU_TEXTUREUSAGE_SAMPLER;
	sunColorInfo.width = 4;
	sunColorInfo.height = 4;
	sunColorInfo.layer_count_or_depth = 1;
	sunColorInfo.num_levels = 1;
	sunColorInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
	renderer->sunColorBuffer = SDL_CreateGPUTexture(device, &sunColorInfo);

	renderer->weather.haziness = 0.01f;
	renderer->weather.cloudCoverage = 0.25f;
	renderer->weather.cloudDensity = 0.1f;
	renderer->weather.windSpeed = 0.05f;
}

void DestroyRenderer(Renderer* renderer)
{
	for (int i = 0; i < TEXTURE_SAMPLER_COUNT; i++)
		SDL_ReleaseGPUSampler(device, renderer->samplers[i]);

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
	renderer->skyTarget = CreateSkyTarget(width / 2, height / 2);

	if (renderer->skyTarget2)
		DestroyRenderTarget(renderer->skyTarget2);
	renderer->skyTarget2 = CreateSkyTarget(width / 2, height / 2);

	if (renderer->shadowBuffer0)
		DestroyRenderTarget(renderer->shadowBuffer0);
	renderer->shadowBuffer0 = CreateShadowBuffer(width / 2, height / 2);

	if (renderer->shadowBuffer1)
		DestroyRenderTarget(renderer->shadowBuffer1);
	renderer->shadowBuffer1 = CreateShadowBuffer(width / 2, height / 2);
}

void RenderMesh(Renderer* renderer, VertexBuffer* vertexBuffers[], int numVertexBuffers, IndexBuffer* indexBuffer, AABB boundingBox, Sphere boundingSphere, Material* material, GraphicsPipeline* shader, bool forward, mat4 transform)
{
	MeshDrawData data = {};

	data.numVertexBuffers = numVertexBuffers;
	SDL_assert(numVertexBuffers <= 16);
	SDL_memcpy(data.vertexBuffers, vertexBuffers, numVertexBuffers * sizeof(VertexBuffer*));

	data.indexBuffer = indexBuffer;

	data.vertexCount = vertexBuffers[0]->numVertices;
	data.indexCount = indexBuffer ? indexBuffer->numIndices : 0;

	data.boundingBox = boundingBox;
	data.boundingSphere = boundingSphere;

	data.material = material;
	data.transform = transform;
	data.shader = shader;

	if (forward)
		renderer->forwardMeshes.add(data);
	else
		renderer->meshes.add(data);
}

void RenderMesh(Renderer* renderer, Mesh* mesh, Material* material, GraphicsPipeline* shader, bool forward, SkeletonState* skeleton, mat4 transform)
{
	MeshDrawData data = {};

	if (skeleton)
	{
		data.numVertexBuffers = NUM_ANIMATED_MESH_BUFFER_LAYOUTS;
		data.vertexBuffers[0] = mesh->positionBuffer;
		data.vertexBuffers[1] = mesh->normalBuffer;
		data.vertexBuffers[2] = mesh->weightsBuffer;
		data.vertexBuffers[3] = mesh->texcoordBuffer;
	}
	else
	{
		data.numVertexBuffers = NUM_MESH_BUFFER_LAYOUTS;
		data.vertexBuffers[0] = mesh->positionBuffer;
		data.vertexBuffers[1] = mesh->normalBuffer;
		data.vertexBuffers[2] = mesh->texcoordBuffer;
	}

	data.indexBuffer = mesh->indexBuffer;

	data.vertexCount = mesh->vertexCount;
	data.indexCount = mesh->indexCount;

	data.boundingBox = mesh->boundingBox;
	data.boundingSphere = mesh->boundingSphere;

	data.skeleton = skeleton;
	data.transform = transform;
	data.material = material;
	data.shader = shader;

	if (forward)
	{
		SDL_assert(!skeleton);

		if (skeleton)
			renderer->forwardMeshes.add(data);
		else
			renderer->forwardMeshes.add(data);
	}
	else
	{
		if (skeleton)
			renderer->animatedMeshes.add(data);
		else
			renderer->meshes.add(data);
	}
}

static void RenderModelNode(Renderer* renderer, Model* model, Node* node, GraphicsPipeline* shader, bool forward, AnimationState* animation, mat4 parentTransform)
{
	mat4 nodeTransform = animation ? GetNodeTransform(animation, node) : node->transform;
	mat4 nodeGlobalTransform = parentTransform * nodeTransform;

	for (int i = 0; i < node->numMeshes; i++)
	{
		int meshID = node->meshes[i];
		Mesh* mesh = &model->meshes[meshID];
		Material* material = mesh->materialID != -1 ? &model->materials[mesh->materialID] : nullptr;
		RenderMesh(renderer, mesh, material, shader, forward, animation && mesh->skeletonID != -1 ? &animation->skeletons[mesh->skeletonID] : nullptr, nodeGlobalTransform);
	}

	for (int i = 0; i < node->numChildren; i++)
	{
		RenderModelNode(renderer, model, &model->nodes[node->children[i]], shader, forward, animation, nodeGlobalTransform);
	}
}

void RenderModel(Renderer* renderer, Model* model, AnimationState* animation, mat4 transform)
{
	SDL_assert(model);
	RenderModelNode(renderer, model, &model->nodes[0], nullptr, false, animation, transform);
}

void RenderModel(Renderer* renderer, Model* model, GraphicsPipeline* shader, AnimationState* animation, mat4 transform)
{
	SDL_assert(model);
	RenderModelNode(renderer, model, &model->nodes[0], shader, shader != nullptr, animation, transform);
}

void RenderLight(Renderer* renderer, vec3 position, vec3 color)
{
	LightDrawData data = {};
	data.position = position;
	data.color = color;
	renderer->pointLights.add(data);
}

void RenderReflectionProbe(Renderer* renderer, ReflectionProbe* probe)
{
	ReflectionProbeDrawData data = {};
	data.probe = probe;
	renderer->reflectionProbes.add(data);
}

void UpdateReflectionProbe(Renderer* renderer, ReflectionProbe* probe)
{
	renderer->reflectionProbeUpdates.add(probe);
}

static void SubmitMesh(Renderer* renderer, VertexBuffer* vertexBuffers[], int numVertexBuffers, IndexBuffer* indexBuffer, int vertexCount, int indexCount, Material* material, SkeletonState* skeleton, const mat4& transform, const mat4& view, const mat4& pv, vec3 cameraPosition, bool viewSpaceBuffer, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmdBuffer)
{
	SDL_GPUBufferBinding vertexBindings[16] = {};
	for (int i = 0; i < numVertexBuffers; i++)
	{
		vertexBindings[i].buffer = vertexBuffers[i] ? vertexBuffers[i]->buffer : renderer->emptyBuffer;
		vertexBindings[i].offset = 0;
	}

	SDL_BindGPUVertexBuffers(renderPass, 0, vertexBindings, numVertexBuffers);

	if (indexBuffer)
	{
		SDL_GPUBufferBinding indexBinding = {};
		indexBinding.buffer = indexBuffer->buffer;
		indexBinding.offset = 0;

		SDL_BindGPUIndexBuffer(renderPass, &indexBinding, indexBuffer->elementSize);
	}

	if (skeleton)
	{
		struct UniformData
		{
			mat4 projectionViewModel;
			mat4 model;
			mat4 boneTransforms[MAX_BONES];
		};

		UniformData uniforms = {};
		uniforms.projectionViewModel = pv * transform;
		uniforms.model = viewSpaceBuffer ? view * transform : transform;
		SDL_memcpy(uniforms.boneTransforms, skeleton->boneTransforms, skeleton->numBones * sizeof(mat4));
		SDL_PushGPUVertexUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));
	}
	else
	{
		struct UniformData
		{
			mat4 projectionViewModel;
			mat4 model;
		};

		UniformData uniforms = {};
		uniforms.projectionViewModel = pv * transform;
		uniforms.model = viewSpaceBuffer ? view * transform : transform;
		SDL_PushGPUVertexUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));
	}

	if (material)
	{
		struct UniformData
		{
			vec4 params;
			vec4 data0;
			vec4 data1;
			vec4 data2;
			vec4 data3;
		};

		UniformData uniforms = {};
		uniforms.params = vec4(cameraPosition, 0);
		uniforms.data0 = material->data0;
		uniforms.data1 = material->data1;
		uniforms.data2 = material->data2;
		uniforms.data3 = material->data3;

		SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		SDL_GPUTextureSamplerBinding textureBindings[MAX_MATERIAL_TEXTURES] = {};
		for (int i = 0; i < material->numTextures; i++)
		{
			textureBindings[i].texture = material->textures[i] ? material->textures[i]->handle : renderer->emptyTexture;
			textureBindings[i].sampler = renderer->samplers[material->samplers[i]];
		}

		SDL_BindGPUFragmentSamplers(renderPass, 0, textureBindings, material->numTextures);
	}

	if (indexBuffer)
		SDL_DrawGPUIndexedPrimitives(renderPass, indexCount, 1, 0, 0, 0);
	else
		SDL_DrawGPUPrimitives(renderPass, vertexCount, 1, 0, 0);
}

static float CalculateLightRadius(vec3 color)
{
	// TODO calculate this based on color and attenuation function
	return 10;
}

// TODO
// [ ] reflection probes
// [ ] point light shadows
// [ ] sunrays
// [ ] particles
// [ ] bloom
// [ ] ambient occlusion
// [ ] TAA
// [ ] render meshes front to back
// [ ] frustum culling
// [ ] reduce cascade updates
// [ ] dont update sky luts
// [ ] reduce sky cubemap updates
// [ ] stencil for light volumes
// [ ] mesh occlusion culling
// [ ] light occlusion culling
// [ ] mesh instancing
// [ ] better pbr (convolution, specular cubemaps)
// [X] atmospheric scattering

void RendererShow(Renderer* renderer, vec3 cameraPosition, quat cameraRotation, float near, float fov, float aspect, mat4 projection, mat4 view, mat4 pv, vec4 frustumPlanes[6], vec3 sunDirection, SDL_GPUTexture* swapchain, SDL_GPUCommandBuffer* cmdBuffer)
{
	GPU_SCOPE("Scene");

	mat4 pvInv = pv.inverted();
	mat4 projectionInv = projection.inverted();
	mat4 viewInv = view.inverted();

	// geometry pass
	{
		GPU_SCOPE("Geometry Pass");

		SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->gbuffer, 0, cmdBuffer);

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->geometryPipeline->pipeline);

		for (int i = 0; i < renderer->meshes.size; i++)
		{
			MeshDrawData* mesh = &renderer->meshes[i];
			SubmitMesh(renderer, mesh->vertexBuffers, mesh->numVertexBuffers, mesh->indexBuffer, mesh->vertexCount, mesh->indexCount, mesh->material, mesh->skeleton, mesh->transform, view, pv, cameraPosition, true, renderPass, cmdBuffer);
		}

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->animatedPipeline->pipeline);

		for (int i = 0; i < renderer->animatedMeshes.size; i++)
		{
			MeshDrawData* mesh = &renderer->animatedMeshes[i];
			SubmitMesh(renderer, mesh->vertexBuffers, mesh->numVertexBuffers, mesh->indexBuffer, mesh->vertexCount, mesh->indexCount, mesh->material, mesh->skeleton, mesh->transform, view, pv, cameraPosition, true, renderPass, cmdBuffer);
		}

		SDL_EndGPURenderPass(renderPass);
	}

	ShadowMapping(renderer, cameraPosition, cameraRotation, near, fov, aspect, projection, view, viewInv, sunDirection, cmdBuffer);

	UpdateSkyCubemap(renderer, cameraPosition, sunDirection, cmdBuffer);

	UpdateReflectionProbes(renderer, sunDirection, cameraPosition, cmdBuffer);

	RenderSky(renderer, projectionInv, viewInv, sunDirection, cmdBuffer);

	// lighting pass
	{
		{
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

		SDL_GPURenderPass* renderPass = BindRenderTarget(renderer->hdrTarget, 0, cmdBuffer);

		// copy depth
		{
			SDL_BindGPUGraphicsPipeline(renderPass, renderer->copyDepthPipeline->pipeline);

			RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 1, &renderer->gbuffer->depthAttachment, &renderer->samplers[TEXTURE_SAMPLER_DEFAULT], cmdBuffer);
		}

		Lighting(renderer, cameraPosition, near, projection, view, pv, projectionInv, viewInv, pvInv, frustumPlanes, sunDirection, renderPass, cmdBuffer);

		// sky upsample
		{
			GPU_TIMER("sky composite");

			SDL_BindGPUGraphicsPipeline(renderPass, renderer->skyUpsamplePipeline->pipeline);

			RenderTarget* rt = app->frameIdx % 2 == 0 ? renderer->skyTarget : renderer->skyTarget2;

			SDL_GPUTexture* textures[3];
			textures[0] = rt->colorAttachments[0];
			textures[1] = renderer->gbuffer->depthAttachment;

			SDL_GPUSampler* samplers[3];
			samplers[0] = renderer->samplers[TEXTURE_SAMPLER_LINEAR_CLAMPED];
			samplers[1] = renderer->samplers[TEXTURE_SAMPLER_CLAMPED];

			RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 2, textures, samplers, cmdBuffer);
		}

		// forward meshes
		{
			GPU_SCOPE("Forward Meshes");

			for (int i = 0; i < renderer->forwardMeshes.size; i++)
			{
				MeshDrawData* mesh = &renderer->forwardMeshes[i];

				SDL_BindGPUGraphicsPipeline(renderPass, mesh->shader->pipeline);

				SubmitMesh(renderer, mesh->vertexBuffers, mesh->numVertexBuffers, mesh->indexBuffer, mesh->vertexCount, mesh->indexCount, mesh->material, mesh->skeleton, mesh->transform, view, pv, cameraPosition, false, renderPass, cmdBuffer);
			}
		}

		SDL_EndGPURenderPass(renderPass);
	}

	AutoExposure(renderer);

	// tonemapping
	{
		GPU_TIMER("tonemap");

		SDL_GPUColorTargetInfo colorTarget = {};
		colorTarget.load_op = SDL_GPU_LOADOP_DONT_CARE;
		colorTarget.store_op = SDL_GPU_STOREOP_STORE;
		colorTarget.texture = swapchain;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdBuffer, &colorTarget, 1, nullptr);

		SDL_BindGPUGraphicsPipeline(renderPass, renderer->tonemappingPipeline->pipeline);

		struct UniformData
		{
			vec4 params;
		};

		UniformData uniforms = {};
		uniforms.params = vec4(renderer->currentExposure, 0, 0, 0);

		SDL_PushGPUFragmentUniformData(cmdBuffer, 0, &uniforms, sizeof(uniforms));

		RenderScreenQuad(&renderer->screenQuad, 1, renderPass, 1, &renderer->hdrTarget->colorAttachments[0], &renderer->samplers[TEXTURE_SAMPLER_DEFAULT], cmdBuffer);

		SDL_EndGPURenderPass(renderPass);
	}

	renderer->lastProjection = projection;
	renderer->lastView = view;

	renderer->meshes.clear();
	renderer->animatedMeshes.clear();
	renderer->forwardMeshes.clear();
	renderer->pointLights.clear();
	renderer->reflectionProbes.clear();
}
