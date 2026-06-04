#include "Trail.h"

#include "Application.h"

#include "graphics/VertexBuffer.h"
#include "graphics/TransferBuffer.h"
#include "graphics/Texture.h"
#include "model/Model.h"

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

void InitTrail(Trail* trail, vec3 position, bool additive, int numNodes)
{
	InitEntity((Entity*)trail, ENTITY_TYPE_TRAIL);

	trail->position = position;
	trail->shader = additive ? game->trailAdditiveShader : game->trailShader;

	trail->width = 0.1f;
	trail->color = vec4(1);

	SDL_assert(numNodes <= MAX_TRAIL_NODES);

	trail->numNodes = numNodes;
	for (int i = 0; i < numNodes; i++)
	{
		trail->nodes[i].position = position;
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

void UpdateTrail(Trail* trail)
{
	if (gameTime - trail->lastNodeUpdate > 1.0f / 60)
	{
		trail->lastNodeUpdate = gameTime;

		for (int i = trail->numNodes - 1; i >= 1; i--)
			trail->nodes[i] = trail->nodes[i - 1];

		trail->nodes[0].position = trail->position;
		trail->nodes[0].distance = trail->nodes[1].distance + (trail->position - trail->nodes[1].position).length();
	}

	if (trail->destroyOnCollapse && trail->nodes[trail->numNodes - 1].position == trail->position)
		trail->removed = true;

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

		float progress = i / (float)(trail->numNodes - 1);

		float width = node->distance == 0 ? 0 : trail->width;
		if (trail->fadeWidth)
			width *= 1 - progress;

		vec4 color = trail->color;
		if (trail->fadeAlpha)
			color.a *= 1 - progress;

		vertex0->position = node->position + right * 0.5f * width;
		vertex0->uv = vec2(node->distance, 0);
		vertex0->color = color;

		vertex1->position = node->position - right * 0.5f * width;
		vertex1->uv = vec2(node->distance, 1);
		vertex1->color = color;
	}

	void* mapped = MapTransferBuffer(trail->transferBuffer);
	SDL_memcpy(mapped, vertices, trail->numNodes * 2 * sizeof(TrailVertex));
	UnmapTransferBuffer(trail->transferBuffer);

	UpdateVertexBuffer(trail->vertexBuffer, 0, trail->numNodes * 2 * sizeof(TrailVertex), trail->transferBuffer->buffer, cmdBuffer);
}

void BendTrailEnd(Trail* trail, vec3 position, float range)
{
	for (int i = 0; i < trail->numNodes; i++)
	{
		TrailNode* node = &trail->nodes[i];
		if (node->distance <= range)
		{
			vec3 forward = i > 0 ? trail->nodes[i - 1].position - node->position : node->position - trail->nodes[i + 1].position;
			forward = forward.normalized();

			vec3 right = cross(forward, vec3::Up);
			right = right.normalized();

			vec3 up = cross(right, forward);
			up = up.normalized();

			vec3 delta = position - node->position;
			vec3 projectedRight = right * dot(right, delta);
			vec3 projectedUp = up * dot(up, delta);

			float bendAmount = 1 - node->distance / range;
			node->position = mix(node->position, node->position + (projectedRight + projectedUp) * bendAmount, 20 * deltaTime);
			if (bendAmount == 1)
				node->position = position;
			//node->position += (projectedRight + projectedUp) * bendAmount;
		}
	}
}

void RenderTrail(Trail* trail)
{
	vec4 params = vec4(trail->texture ? 1.0f : 0.0f, 0, 0, 0);

	RenderMesh(&game->renderer, &trail->vertexBuffer, 1, nullptr, trail->numNodes * 2, 1, {}, {}, &params, sizeof(params), &trail->texture, &trail->textureSampler, 1, trail->shader, mat4::Identity);
}
