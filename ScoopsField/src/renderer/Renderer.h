#pragma once

#include "ScreenQuad.h"

#include "graphics/RenderTarget.h"
#include "graphics/Shader.h"
#include "graphics/GraphicsPipeline.h"
#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/TransferBuffer.h"

#include "model/Model.h"
#include "model/Animation.h"

#include "math/Matrix.h"

#include "utils/List.h"

#include <SDL3/SDL.h>


struct MeshDrawData
{
	Mesh* mesh;
	Material* material;
	SkeletonState* skeleton;
	mat4 transform;
};

struct LightDrawData
{
	vec3 position;
	vec3 color;
};

struct WorldDrawData
{
	vec3 sunDirection;
};

struct Renderer
{
	int width, height;

	SDL_GPUTexture* depthTexture;
	RenderTarget* gbuffer;
	RenderTarget* hdrTarget;
	RenderTarget* skyTarget;
	RenderTarget* skyTarget2;

#define NUM_MESH_BUFFER_LAYOUTS 3
	VertexBufferLayout meshLayout[NUM_MESH_BUFFER_LAYOUTS];
#define NUM_ANIMATED_MESH_BUFFER_LAYOUTS 4
	VertexBufferLayout animatedLayout[NUM_ANIMATED_MESH_BUFFER_LAYOUTS];

	VertexBuffer* cubeVertexBuffer;
	IndexBuffer* cubeIndexBuffer;
	VertexBuffer* pointLightInstanceBuffer;
	TransferBuffer* pointLightInstanceTransferBuffer;

	ScreenQuad screenQuad;

	Shader* defaultShader;
	Shader* animatedShader;
	Shader* copyDepthShader;
	Shader* directionalLightShader;
	Shader* pointLightShader;
	Shader* environmentLightShader;
	Shader* skyShader;
	Shader* skyUpsampleShader;
	Shader* tonemappingShader;

	GraphicsPipeline* geometryPipeline;
	GraphicsPipeline* animatedPipeline;
	GraphicsPipeline* copyDepthPipeline;
	GraphicsPipeline* directionalLightPipeline;
	GraphicsPipeline* pointLightPipeline;
	GraphicsPipeline* environmentLightPipeline;
	GraphicsPipeline* skyPipeline;
	GraphicsPipeline* skyUpsamplePipeline;
	GraphicsPipeline* tonemappingPipeline;

	SDL_GPUSampler* defaultSampler;
	SDL_GPUSampler* linearSampler;
	SDL_GPUBuffer* emptyBuffer;
	SDL_GPUTexture* emptyTexture;

#define MAX_MESH_DRAWS 1024
	List<MeshDrawData, MAX_MESH_DRAWS> meshes;
#define MAX_ANIMATED_MESH_DRAWS 64
	List<MeshDrawData, MAX_MESH_DRAWS> animatedMeshes;
#define MAX_POINT_LIGHT_DRAWS 256
	List<LightDrawData, MAX_POINT_LIGHT_DRAWS> pointLights;

	Texture* valueNoise3D;
	Texture* blueNoise;

	Texture* environmentMap;
};


void InitRenderer(Renderer* renderer, int width, int height, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyRenderer(Renderer* renderer);
void ResizeRenderer(Renderer* renderer, int width, int height);

void RenderModel(Renderer* renderer, Model* model, AnimationState* animation, mat4 transform);
void RenderLight(Renderer* renderer, vec3 position, vec3 color);

void RendererShow(Renderer* renderer, vec3 cameraPosition, mat4 projection, mat4 view, mat4 pv, vec4 frustumPlanes[6], float near, vec3 sunDirection, SDL_GPUTexture* swapchain, SDL_GPUCommandBuffer* cmdBuffer);
