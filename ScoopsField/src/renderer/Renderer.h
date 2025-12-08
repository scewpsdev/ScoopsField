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
	mat4 transform;
	SkeletonState* skeleton;
};

struct LightDrawData
{
	vec3 position;
	vec3 color;
};

struct Renderer
{
	int width, height;

	SDL_GPUTexture* depthTexture;
	RenderTarget* gbuffer;
	RenderTarget* hdrTarget;

#define MAX_MESH_DRAWS 1024
	List<MeshDrawData, MAX_MESH_DRAWS> meshes;
#define MAX_ANIMATED_MESH_DRAWS 64
	List<MeshDrawData, MAX_MESH_DRAWS> animatedMeshes;
#define MAX_POINT_LIGHT_DRAWS 256
	List<LightDrawData, MAX_POINT_LIGHT_DRAWS> pointLights;

	VertexBufferLayout meshLayout[2];
	VertexBufferLayout animatedLayout[3];

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
	Shader* tonemappingShader;

	GraphicsPipeline* geometryPipeline;
	GraphicsPipeline* animatedPipeline;
	GraphicsPipeline* copyDepthPipeline;
	GraphicsPipeline* directionalLightPipeline;
	GraphicsPipeline* pointLightPipeline;
	GraphicsPipeline* tonemappingPipeline;

	SDL_GPUSampler* defaultSampler;
	SDL_GPUBuffer* emptyBuffer;
};


void InitRenderer(Renderer* renderer, int width, int height, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyRenderer(Renderer* renderer);
void ResizeRenderer(Renderer* renderer, int width, int height);

void RenderMesh(Renderer* renderer, Mesh* mesh, SkeletonState* skeleton, mat4 transform);
void RenderModel(Renderer* renderer, Model* model, AnimationState* animation, mat4 transform);
void RenderLight(Renderer* renderer, vec3 position, vec3 color);

void RendererShow(Renderer* renderer, mat4 projection, mat4 view, float near, float far, SDL_GPUCommandBuffer* cmdBuffer);
