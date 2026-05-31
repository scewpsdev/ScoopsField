#pragma once

#include "ScreenQuad.h"
#include "ReflectionProbe.h"

#include "graphics/RenderTarget.h"
#include "graphics/Shader.h"
#include "graphics/GraphicsPipeline.h"
#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/TransferBuffer.h"
#include "graphics/StorageBuffer.h"

#include "model/Model.h"
#include "model/Animation.h"

#include "math/Matrix.h"

#include "utils/List.h"

#include <SDL3/SDL.h>


struct MeshDrawData
{
	VertexBuffer* vertexBuffers[16];
	int numVertexBuffers;

	IndexBuffer* indexBuffer;

	int vertexCount, indexCount;

	AABB boundingBox;
	Sphere boundingSphere;

	SkeletonState* skeleton;
	Material* material;
	GraphicsPipeline* shader;
	mat4 transform;
};

struct LightDrawData
{
	vec3 position;
	vec3 color;
};

struct ReflectionProbeDrawData
{
	ReflectionProbe* probe;
};

struct WorldDrawData
{
	vec3 sunDirection;
};

struct WeatherData
{
	float haziness;
	float cloudCoverage;
	float cloudDensity;
	float windSpeed;

	vec4 getData() const
	{
		return vec4(haziness, cloudCoverage, cloudDensity, windSpeed);
	}
};

struct Renderer
{
	int width, height;

	mat4 lastProjection, lastView;

	SDL_GPUTexture* depthTexture;
	RenderTarget* gbuffer;
	RenderTarget* hdrTarget;

#define NUM_MESH_BUFFER_LAYOUTS 3
	VertexBufferLayout meshLayout[NUM_MESH_BUFFER_LAYOUTS];
#define NUM_ANIMATED_MESH_BUFFER_LAYOUTS 4
	VertexBufferLayout animatedLayout[NUM_ANIMATED_MESH_BUFFER_LAYOUTS];

	VertexBuffer* cubeVertexBuffer;
	IndexBuffer* cubeIndexBuffer;
	ScreenQuad screenQuad;

	VertexBuffer* pointLightInstanceBuffer;
	TransferBuffer* pointLightInstanceTransferBuffer;

	RenderTarget* shadowMaps[3];
	RenderTarget* shadowBuffer0;
	RenderTarget* shadowBuffer1;
	Shader* depthShader;
	Shader* animatedDepthShader;
	Shader* shadowShader;
	Shader* blurHShader;
	Shader* blurVShader;
	GraphicsPipeline* shadowMapPipeline;
	GraphicsPipeline* animatedShadowMapPipeline;
	GraphicsPipeline* shadowPipeline;
	GraphicsPipeline* blurHPipeline;
	GraphicsPipeline* blurVPipeline;

	RenderTarget* cubemapGbuffers[6];
	RenderTarget* reflectionProbeShadowMap;

	Shader* defaultShader;
	Shader* animatedShader;
	Shader* copyDepthShader;
	Shader* directionalLightShader;
	Shader* pointLightShader;
	Shader* environmentLightShader;
	Shader* reflectionProbeShader;
	Shader* deferredDiffuseShader;
	Shader* shConvoluteShader;
	Shader* tonemappingShader;

	GraphicsPipeline* geometryPipeline;
	GraphicsPipeline* animatedPipeline;
	GraphicsPipeline* copyDepthPipeline;
	GraphicsPipeline* copyDepthPipeline2;
	GraphicsPipeline* directionalLightPipeline;
	GraphicsPipeline* pointLightPipeline;
	GraphicsPipeline* environmentLightPipeline;
	GraphicsPipeline* reflectionProbePipeline;
	GraphicsPipeline* deferredDiffusePipeline;
	GraphicsPipeline* tonemappingPipeline;

	RenderTarget* skyTarget;
	RenderTarget* skyTarget2;
	RenderTarget* skyCubemap;
	SDL_GPUTexture* skyTransmittanceLUT;
	SDL_GPUTexture* skyMultiScatterLUT;
	SDL_GPUTexture* skyViewLUT;
	SDL_GPUTexture* skyAerialLUT;
	SDL_GPUTexture* cloudNoise;
	SDL_GPUTexture* cloudNoiseDetail;
	SDL_GPUTexture* sunColorBuffer;
	Shader* skyShader;
	Shader* skyUpsampleShader;
	Shader* skyCubeShader;
	Shader* skyTransmittanceLUTShader;
	Shader* skyMultiScatterLUTShader;
	Shader* skyViewLUTShader;
	Shader* skyAerialLUTShader;
	Shader* cloudNoiseShader;
	Shader* cloudNoiseDetailShader;
	Shader* sunColorShader;
	GraphicsPipeline* skyPipeline;
	GraphicsPipeline* skyUpsamplePipeline;
	GraphicsPipeline* skyCubePipeline;

	SDL_GPUSampler* samplers[TEXTURE_SAMPLER_COUNT];
	SDL_GPUBuffer* emptyBuffer;
	SDL_GPUTexture* emptyTexture;

	SDL_GPUFence* luminanceReadbackFence;
	SDL_GPUTransferBuffer* luminanceReadbackBuffer;
	float targetExposure;
	float currentExposure;

#define MAX_MESH_DRAWS 1024
	List<MeshDrawData, MAX_MESH_DRAWS> meshes;
#define MAX_ANIMATED_MESH_DRAWS 64
	List<MeshDrawData, MAX_MESH_DRAWS> animatedMeshes;
#define MAX_FORWARD_MESH_DRAWS 64
	List<MeshDrawData, MAX_FORWARD_MESH_DRAWS> forwardMeshes;

#define MAX_POINT_LIGHT_DRAWS 256
	List<LightDrawData, MAX_POINT_LIGHT_DRAWS> pointLights;
#define MAX_REFLECTION_PROBES 16
	List<ReflectionProbeDrawData, MAX_REFLECTION_PROBES> reflectionProbes;
	List<ReflectionProbe*, MAX_REFLECTION_PROBES> reflectionProbeUpdates;

	Texture* blueNoise;
	//Texture* environmentMap;

	WeatherData weather;
};


void RenderMesh(Renderer* renderer, VertexBuffer* vertexBuffers[], int numVertexBuffers, IndexBuffer* indexBuffer, AABB boundingBox, Sphere boundingSphere, Material* material, GraphicsPipeline* shader, bool forward, mat4 transform);
void RenderModel(Renderer* renderer, Model* model, AnimationState* animation, mat4 transform);
void RenderModel(Renderer* renderer, Model* model, GraphicsPipeline* shader, AnimationState* animation, mat4 transform);
void RenderLight(Renderer* renderer, vec3 position, vec3 color);
void RenderReflectionProbe(Renderer* renderer, ReflectionProbe* probe);
void UpdateReflectionProbe(Renderer* renderer, ReflectionProbe* probe);
