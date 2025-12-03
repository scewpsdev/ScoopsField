#include "Mesh.h"

#include "Application.h"

#include "math/Shape.h"

#include "utils/BinaryReader.h"


extern GameMemory* memory;
extern ResourceState* resource;


void InitMesh(Mesh* mesh, int vertexCount, const vec3* positions, const vec3* normals, int indexCount, const uint8_t* indices, SDL_GPUIndexElementSize indexElementSize, SDL_GPUCommandBuffer* cmdBuffer)
{
	mesh->vertexCount = vertexCount;
	mesh->indexCount = indexCount;

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

	VertexBufferLayout positionLayout = {};
	positionLayout.numAttributes = 1;
	positionLayout.attributes[0].location = 0;
	positionLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;

	mesh->positionBuffer = CreateVertexBuffer(vertexCount, &positionLayout, 0);
	UpdateVertexBuffer(mesh->positionBuffer, 0, (const uint8_t*)positions, vertexCount * sizeof(vec3), copyPass);

	VertexBufferLayout normalLayout = {};
	normalLayout.numAttributes = 1;
	normalLayout.attributes[0].location = 1;
	normalLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;

	mesh->normalBuffer = CreateVertexBuffer(vertexCount, &normalLayout, 0);
	UpdateVertexBuffer(mesh->normalBuffer, 0, (const uint8_t*)normals, vertexCount * sizeof(vec3), copyPass);

	mesh->indexBuffer = CreateIndexBuffer(indexCount, indexElementSize);
	UpdateIndexBuffer(mesh->indexBuffer, 0, indices, indexCount * GetIndexFormatSize(indexElementSize), copyPass);

	SDL_EndGPUCopyPass(copyPass);
}

static void ReadMesh(BinaryReader& reader, Mesh* mesh, SDL_GPUCommandBuffer* cmdBuffer)
{
	int hasPositions = reader.ReadInt32();
	int hasNormals = reader.ReadInt32();
	int hasTangents = reader.ReadInt32();
	int hasTexCoords = reader.ReadInt32();
	int hasVertexColors = reader.ReadInt32();
	int hasBones = reader.ReadInt32();

	int vertexCount = reader.ReadInt32();

	vec3* positions = nullptr, * normals = nullptr;

	if (hasPositions)
	{
		positions = (vec3*)BumpAllocatorMalloc(&memory->transientAllocator, vertexCount * sizeof(vec3));
		reader.ReadBytes(positions, vertexCount * sizeof(vec3));
	}

	if (hasNormals)
	{
		normals = (vec3*)BumpAllocatorMalloc(&memory->transientAllocator, vertexCount * sizeof(vec3));
		reader.ReadBytes(normals, vertexCount * sizeof(vec3));
	}

	if (hasTangents)
		reader.Skip(vertexCount * sizeof(vec3));

	if (hasTexCoords)
		reader.Skip(vertexCount * sizeof(vec2));

	if (hasVertexColors)
		reader.Skip(vertexCount * sizeof(uint32_t));

	if (hasBones)
		reader.Skip(vertexCount * 2 * sizeof(vec4));

	int indexCount = reader.ReadInt32();

	SDL_GPUIndexElementSize indexElementSize = vertexCount < UINT16_MAX ? SDL_GPU_INDEXELEMENTSIZE_16BIT : SDL_GPU_INDEXELEMENTSIZE_32BIT;

	uint8_t* indices = BumpAllocatorMalloc(&memory->transientAllocator, indexCount * GetIndexFormatSize(indexElementSize));
	for (int i = 0; i < indexCount; i++)
	{
		if (indexElementSize == SDL_GPU_INDEXELEMENTSIZE_16BIT)
			((uint16_t*)indices)[i] = (uint16_t)reader.ReadUInt32();
		else
			((uint32_t*)indices)[i] = reader.ReadUInt32();
	}

	int materialID = reader.ReadInt32();
	int skeletonID = reader.ReadInt32();

	AABB boundingBox;
	reader.Read(&boundingBox);

	Sphere boundingSphere;
	reader.Read(&boundingSphere);

	InitMesh(mesh, vertexCount, positions, normals, indexCount, indices, indexElementSize, cmdBuffer);
}

static void ReadTexture(BinaryReader& reader)
{
	char path[256];
	reader.ReadBytes(path, sizeof(path));

	int isEmbedded = reader.ReadInt32();

	if (isEmbedded)
	{
		int width = reader.ReadInt32();
		int height = reader.ReadInt32();

		if (!height)
			reader.Skip(width);
		else
			reader.Skip(width * height * sizeof(uint32_t));
	}
}

static void ReadMaterial(BinaryReader& reader)
{
	uint32_t color = reader.ReadUInt32();
	float metallicFactor = reader.ReadFloat();
	float roughnessFactor = reader.ReadFloat();
	vec3 emissiveColor = reader.ReadVector3();
	float emissiveStrength = reader.ReadFloat();

	int hasDiffuse = reader.ReadInt32();
	int hasNormal = reader.ReadInt32();
	int hasRoughness = reader.ReadInt32();
	int hasMetallic = reader.ReadInt32();
	int hasEmissive = reader.ReadInt32();
	int hasHeight = reader.ReadInt32();

	if (hasDiffuse) ReadTexture(reader);
	if (hasNormal) ReadTexture(reader);
	if (hasRoughness) ReadTexture(reader);
	if (hasMetallic) ReadTexture(reader);
	if (hasEmissive) ReadTexture(reader);
	if (hasHeight) ReadTexture(reader);
}

static void ReadSkeleton(BinaryReader& reader)
{
	int boneCount = reader.ReadInt32();

	for (int i = 0; i < boneCount; i++)
	{
		char name[32];
		reader.ReadBytes(name, sizeof(name));

		mat4 offsetMatrix = reader.ReadMatrix4();
		int nodeID = reader.ReadInt32();
	}
}

static void ReadAnimation(BinaryReader& reader)
{
	char name[32];
	reader.ReadBytes(name, sizeof(name));

	float duration = reader.ReadFloat();

	int numPositions = reader.ReadInt32();
	int numRotations = reader.ReadInt32();
	int numScalings = reader.ReadInt32();
	int numChannels = reader.ReadInt32();

	reader.Skip(numPositions * sizeof(PositionKeyframe));
	reader.Skip(numRotations * sizeof(RotationKeyframe));
	reader.Skip(numScalings * sizeof(ScalingKeyframe));

	for (int i = 0; i < numChannels; i++)
	{
		char name[32];
		reader.ReadBytes(name, sizeof(name));

		int positionsOffset = reader.ReadInt32();
		int positionsCount = reader.ReadInt32();
		int rotationsOffset = reader.ReadInt32();
		int rotationsCount = reader.ReadInt32();
		int scalingsOffset = reader.ReadInt32();
		int scalingsCount = reader.ReadInt32();
	}
}

static void ReadNode(BinaryReader& reader)
{
	int nodeID = reader.ReadInt32();

	char name[64];
	reader.ReadBytes(name, sizeof(name));

	int armatureID = reader.ReadInt32();

	mat4 transform = reader.ReadMatrix4();

	int numChildren = reader.ReadInt32();
	int numMeshes = reader.ReadInt32();

	reader.Skip(numChildren * sizeof(int));
	reader.Skip(numMeshes * sizeof(int));

	int numProperties = reader.ReadInt32();
	for (int i = 0; i < numProperties; i++)
	{
		char name[32];
		reader.ReadBytes(name, sizeof(name));

		int propertyType = reader.ReadInt32();
		if (propertyType <= 4)
		{
			uint64_t value = reader.ReadUInt64();
		}
		else if (propertyType == 5)
		{
			int len = reader.ReadInt32();
			len = (len * 4 + 3) / 4;
			reader.Skip(len);
		}
		else if (propertyType == 6)
		{
			reader.ReadVector3();
		}
	}
}

static void ReadLight(BinaryReader& reader)
{
	char name[32];
	reader.ReadBytes(name, sizeof(name));

	int type = reader.ReadInt32();
	vec3 position = reader.ReadVector3();
	vec3 direction = reader.ReadVector3();
	vec3 color = reader.ReadVector3();
}

Mesh* LoadMesh(const char* path, SDL_GPUCommandBuffer* cmdBuffer)
{
	// TODO free memory
	size_t fileSize;
	void* data = SDL_LoadFile(path, &fileSize);

	BinaryReader reader((uint8_t*)data, (int)fileSize);

	SDL_assert(resource->numMeshes < MAX_MESHES);
	Mesh* mesh = &resource->meshes[resource->numMeshes++];

	int numMeshes = reader.ReadInt32();
	int numMaterials = reader.ReadInt32();
	int numSkeletons = reader.ReadInt32();
	int numAnimations = reader.ReadInt32();
	int numNodes = reader.ReadInt32();
	int numLights = reader.ReadInt32();

	SDL_assert(numMeshes > 0);

	for (int i = 0; i < numMeshes; i++)
		ReadMesh(reader, mesh, cmdBuffer);

	for (int i = 0; i < numMaterials; i++)
		ReadMaterial(reader);

	for (int i = 0; i < numSkeletons; i++)
		ReadSkeleton(reader);

	for (int i = 0; i < numAnimations; i++)
		ReadAnimation(reader);

	for (int i = 0; i < numNodes; i++)
		ReadNode(reader);

	for (int i = 0; i < numLights; i++)
		ReadLight(reader);

	AABB boundingBox;
	reader.Read(&boundingBox);

	Sphere boundingSphere;
	reader.Read(&boundingSphere);

	return mesh;
}
