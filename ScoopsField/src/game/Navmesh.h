#pragma once

#include "math/Vector.h"
#include "math/Matrix.h"

#include "utils/HashSet.h"


#define MAX_NAVMESH_PATH_LENGTH 64

struct NavmeshNode
{
	int layer;
	vec3 position;
#define MAX_NAVMESH_NODE_CONNECTIONS 8
	HashSet<int, MAX_NAVMESH_NODE_CONNECTIONS> connections;

	bool visitedFlag;
};

struct Navmesh
{
#define MAX_NAVMESH_NODES 64
	NavmeshNode nodes[MAX_NAVMESH_NODES];
	int numNodes;
};


void InitNavmesh(Navmesh* navmesh, struct Model* model);
bool CalculateNavmeshPath(Navmesh* navmesh, const vec3& from, const vec3& to, int path[MAX_NAVMESH_PATH_LENGTH], int& pathLength);
