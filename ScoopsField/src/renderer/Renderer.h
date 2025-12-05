#pragma once

#include "ScreenQuad.h"

#include "graphics/RenderTarget.h"
#include "graphics/Shader.h"
#include "graphics/GraphicsPipeline.h"
#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/TransferBuffer.h"

#include "model/Mesh.h"

#include "math/Matrix.h"

#include "utils/List.h"

#include <SDL3/SDL.h>


struct MeshDrawData
{
	Mesh* mesh;
	mat4 transform;
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

	Shader* defaultShader;
	GraphicsPipeline* geometryPipeline;

#define MAX_MESH_DRAWS 1024
	List<MeshDrawData, MAX_MESH_DRAWS> meshes;

	RenderTarget* hdrTarget;

	Shader* copyDepthShader;
	GraphicsPipeline* copyDepthPipeline;

	Shader* directionalLightShader;
	GraphicsPipeline* directionalLightPipeline;

	Shader* pointLightShader;
	GraphicsPipeline* pointLightPipeline;
	VertexBuffer* cubeVertexBuffer;
	IndexBuffer* cubeIndexBuffer;
	VertexBuffer* pointLightInstanceBuffer;
	TransferBuffer* pointLightInstanceTransferBuffer;

#define MAX_POINT_LIGHT_DRAWS 256
	List<LightDrawData, MAX_POINT_LIGHT_DRAWS> pointLights;

	Shader* tonemappingShader;
	GraphicsPipeline* tonemappingPipeline;

	ScreenQuad screenQuad;
	SDL_GPUSampler* defaultSampler;
};


void InitRenderer(Renderer* renderer, int width, int height, SDL_GPUCommandBuffer* cmdBuffer);
void DestroyRenderer(Renderer* renderer);
void ResizeRenderer(Renderer* renderer, int width, int height);

void RenderMesh(Renderer* renderer, Mesh* mesh, mat4 transform);
void RenderLight(Renderer* renderer, vec3 position, vec3 color);

void RendererShow(Renderer* renderer, mat4 projection, mat4 view, float near, float far, SDL_GPUCommandBuffer* cmdBuffer);
