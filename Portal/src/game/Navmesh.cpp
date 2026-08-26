#include "Navmesh.h"

#include "GameMemory.h"

#include "model/Model.h"

#include "utils/StringUtils.h"

#include <float.h>


extern GameMemory* memory;


static void InitNavmesh(Navmesh* navmesh, Mesh* mesh, int layer, const mat4& transform)
{
	for (int i = 0; i < mesh->vertexCount; i++)
	{
		SDL_assert(navmesh->numNodes < MAX_NAVMESH_NODES);
		NavmeshNode* node = &navmesh->nodes[navmesh->numNodes++];
		node->layer = layer;
		node->position = transform * mesh->cachedPositions[i];
		InitHashSet(&node->connections);
	}

	SDL_assert(mesh->indexCount % 3 == 0);
	SDL_assert(mesh->indexBuffer->elementSize == SDL_GPU_INDEXELEMENTSIZE_16BIT);
	uint16_t* indices = (uint16_t*)mesh->cachedIndices;
	for (int i = 0; i < mesh->indexCount / 3; i++)
	{
		int i0 = indices[i * 3 + 0];
		int i1 = indices[i * 3 + 1];
		int i2 = indices[i * 3 + 2];
		SDL_assert(i0 != UINT16_MAX && i1 != UINT16_MAX && i2 == UINT16_MAX);

		NavmeshNode* node0 = &navmesh->nodes[i0];
		NavmeshNode* node1 = &navmesh->nodes[i1];

		//SDL_assert(node0->numConnections < MAX_NAVMESH_NODE_CONNECTIONS);
		//SDL_assert(node1->numConnections < MAX_NAVMESH_NODE_CONNECTIONS);

		HashSetAdd(&node0->connections, i1);
		HashSetAdd(&node1->connections, i0);
		//node0->connections[node0->numConnections++] = i1;
		//node1->connections[node1->numConnections++] = i0;
	}
}

void InitNavmesh(Navmesh* navmesh, Model* model)
{
	*navmesh = {};
	for (int i = 0; i < model->numMeshes; i++)
	{
		InitNavmesh(navmesh, &model->meshes[i], i, mat4::Identity);
	}
}

static int GetClosestNode(Navmesh* navmesh, const vec3& position)
{
	int closest = -1;
	float closestDistanceSq = FLT_MAX;
	for (int i = 0; i < navmesh->numNodes; i++)
	{
		float distanceSq = (navmesh->nodes[i].position - position).lengthSquared();
		if (distanceSq < closestDistanceSq)
		{
			closest = i;
			closestDistanceSq = distanceSq;
		}
	}

	SDL_assert(closest != -1);
	return closest;
}

struct AStarNode
{
	float g;
	float f;
	int parent;
	bool open;
	bool closed;
};

static bool CalculateNavmeshPath(Navmesh* navmesh, int start, int goal, int path[MAX_NAVMESH_PATH_LENGTH], int& pathLength)
{
	AStarNode data[MAX_NAVMESH_NODES];

	for (int i = 0; i < navmesh->numNodes; i++)
	{
		data[i].g = FLT_MAX;
		data[i].f = FLT_MAX;
		data[i].parent = -1;
		data[i].open = false;
		data[i].closed = false;
	}

	data[start].g = 0;
	data[start].f = (navmesh->nodes[start].position - navmesh->nodes[goal].position).length();
	data[start].open = true;

	while (true)
	{
		int current = -1;
		float bestF = FLT_MAX;

		for (int i = 0; i < navmesh->numNodes; i++)
		{
			if (data[i].open && data[i].f < bestF)
			{
				bestF = data[i].f;
				current = i;
			}
		}

		if (current == -1)
			return false;

		if (current == goal)
			break;

		data[current].open = false;
		data[current].closed = true;

		NavmeshNode* node = &navmesh->nodes[current];

		for (int i = 0; i < node->connections.capacity; i++)
		{
			if (node->connections.slots[i].state == HASHSET_SLOT_USED)
			{
				int neighbor = node->connections.slots[i].value;
				if (data[neighbor].closed)
					continue;

				float cost = (node->position - navmesh->nodes[neighbor].position).length();
				float tentativeG = data[current].g + cost;

				if (!data[neighbor].open || tentativeG < data[neighbor].g)
				{
					data[neighbor].parent = current;
					data[neighbor].g = tentativeG;
					data[neighbor].f = tentativeG + (navmesh->nodes[neighbor].position - navmesh->nodes[goal].position).length();
					data[neighbor].open = true;
				}
			}
		}
	}

	pathLength = 0;
	int current = goal;
	while (current != -1)
	{
		path[pathLength++] = current;
		current = data[current].parent;
	}

	ReverseArray(path, sizeof(int), pathLength);

	return true;
}

bool CalculateNavmeshPath(Navmesh* navmesh, const vec3& from, const vec3& to, int path[MAX_NAVMESH_PATH_LENGTH], int& pathLength)
{
	int fromNode = GetClosestNode(navmesh, from);
	int toNode = GetClosestNode(navmesh, to);

	if (fromNode == toNode)
	{
		pathLength = 0;
		return true;
	}

	return CalculateNavmeshPath(navmesh, fromNode, toNode, path, pathLength);
}
