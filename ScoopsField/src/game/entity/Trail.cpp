#include "Trail.h"

#include "Application.h"

#include "graphics/VertexBuffer.h"
#include "graphics/TransferBuffer.h"

#include "renderer/Renderer.h"


struct TrailVertex
{
	vec3 position;
	vec2 uv;
	vec4 color;
};


void InitTrailVertexLayout(VertexBufferLayout* layout)
{
	layout->numAttributes = 3;
	layout->attributes[0].location = 0;
	layout->attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	layout->attributes[1].location = 1;
	layout->attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	layout->attributes[2].location = 2;
	layout->attributes[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
}

void InitTrail(Trail* trail, float width, vec3 startPosition, int numNodes)
{
	trail->width = width;

	SDL_assert(numNodes <= MAX_TRAIL_NODES);

	trail->numNodes = numNodes;
	for (int i = 0; i < numNodes; i++)
	{
		trail->nodes[i].position = startPosition;
		trail->nodes[i].distance = 0;
	}

	VertexBufferLayout layout = {};
	InitTrailVertexLayout(&layout);

	trail->vertexBuffer = CreateVertexBuffer(numNodes * 2, &layout, 0);

	trail->transferBuffer = CreateTransferBuffer(numNodes * 2 * sizeof(TrailVertex), SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, true);

	trail->lastNodeUpdate = 0;
}

void DestroyTrail(Trail* trail)
{
	DestroyVertexBuffer(trail->vertexBuffer);
	DestroyTransferBuffer(trail->transferBuffer);
}

void UpdateTrail(Trail* trail, vec3 position, SDL_GPUCommandBuffer* cmdBuffer)
{
	if (gameTime - trail->lastNodeUpdate > 1.0f / 60)
	{
		trail->lastNodeUpdate = gameTime;

		for (int i = trail->numNodes - 1; i >= 1; i--)
			trail->nodes[i] = trail->nodes[i - 1];

		trail->nodes[0].position = position;
		trail->nodes[0].distance = trail->nodes[1].distance + (position - trail->nodes[1].position).length();
	}

	TrailVertex vertices[MAX_TRAIL_NODES * 2];

	for (int i = 0; i < trail->numNodes; i++)
	{
		TrailNode* node = &trail->nodes[i];

		vec3 forward = i > 0 ? trail->nodes[i - 1].position - node->position : node->position - trail->nodes[i + 1].position;
		forward = forward.normalized();

		vec3 toCamera = game->cameraPosition - node->position;
		toCamera = toCamera.normalized();

		vec3 right = cross(forward, toCamera);
		right = right.normalized();

		TrailVertex* vertex0 = &vertices[i * 2 + 0];
		TrailVertex* vertex1 = &vertices[i * 2 + 1];

		float width = (1 - i / (float)(trail->numNodes - 1)) * trail->width;

		vertex0->position = node->position - right * 0.5f * width;
		vertex0->uv = vec2(node->distance, 0);
		vertex0->color = vec4(1);

		vertex1->position = node->position + right * 0.5f * width;
		vertex1->uv = vec2(node->distance, 1);
		vertex1->color = vec4(1);
	}

	void* mapped = MapTransferBuffer(trail->transferBuffer);
	SDL_memcpy(mapped, vertices, trail->numNodes * 2 * sizeof(TrailVertex));
	UnmapTransferBuffer(trail->transferBuffer);

	UpdateVertexBuffer(trail->vertexBuffer, 0, trail->numNodes * 2 * sizeof(TrailVertex), trail->transferBuffer->buffer, cmdBuffer);
}

void RenderTrail(Trail* trail)
{
	RenderMesh(&game->renderer, &trail->vertexBuffer, 1, nullptr, {}, {}, nullptr, game->trailShader, true, mat4::Identity);
}
