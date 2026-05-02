#include "Model.h"

#include "Application.h"

#include "math/Shape.h"

#include "utils/BinaryReader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "utils/stb_image.h"


extern GameMemory* memory;
extern ResourceState* resource;


void InitMesh(Mesh* mesh, int vertexCount, const vec3* positions, const vec3* normals, const vec4* weights, const vec2* texcoords, int indexCount, const uint8_t* indices, SDL_GPUIndexElementSize indexElementSize, SDL_GPUCommandBuffer* cmdBuffer)
{
	mesh->vertexCount = vertexCount;
	mesh->indexCount = indexCount;

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

	if (positions)
	{
		VertexBufferLayout positionLayout = {};
		positionLayout.numAttributes = 1;
		positionLayout.attributes[0].location = 0;
		positionLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;

		mesh->positionBuffer = CreateVertexBuffer(vertexCount, &positionLayout, 0);
		UpdateVertexBuffer(mesh->positionBuffer, 0, (const uint8_t*)positions, vertexCount * sizeof(vec3), copyPass);
	}

	if (normals)
	{
		VertexBufferLayout normalLayout = {};
		normalLayout.numAttributes = 1;
		normalLayout.attributes[0].location = 1;
		normalLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;

		mesh->normalBuffer = CreateVertexBuffer(vertexCount, &normalLayout, 0);
		UpdateVertexBuffer(mesh->normalBuffer, 0, (const uint8_t*)normals, vertexCount * sizeof(vec3), copyPass);
	}

	if (weights)
	{
		VertexBufferLayout weightsLayout = {};
		weightsLayout.numAttributes = 2;
		weightsLayout.attributes[0].location = 2;
		weightsLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
		weightsLayout.attributes[1].location = 3;
		weightsLayout.attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;

		mesh->weightsBuffer = CreateVertexBuffer(vertexCount, &weightsLayout, 0);
		UpdateVertexBuffer(mesh->weightsBuffer, 0, (const uint8_t*)weights, vertexCount * 2 * sizeof(vec4), copyPass);
	}

	if (texcoords)
	{
		VertexBufferLayout texcoordLayout = {};
		texcoordLayout.numAttributes = 1;
		texcoordLayout.attributes[0].location = 4;
		texcoordLayout.attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;

		mesh->texcoordBuffer = CreateVertexBuffer(vertexCount, &texcoordLayout, 0);
		UpdateVertexBuffer(mesh->texcoordBuffer, 0, (const uint8_t*)texcoords, vertexCount * sizeof(vec2), copyPass);
	}

	if (indices)
	{
		mesh->indexBuffer = CreateIndexBuffer(indexCount, indexElementSize);
		UpdateIndexBuffer(mesh->indexBuffer, 0, indices, indexCount * GetIndexFormatSize(indexElementSize), copyPass);
	}

	SDL_EndGPUCopyPass(copyPass);
}

static void ReadMesh(Mesh* mesh, BinaryReader& reader, bool cacheData, SDL_GPUCommandBuffer* cmdBuffer)
{
	int hasPositions = reader.ReadInt32();
	int hasNormals = reader.ReadInt32();
	int hasTangents = reader.ReadInt32();
	int hasTexCoords = reader.ReadInt32();
	int hasVertexColors = reader.ReadInt32();
	int hasBones = reader.ReadInt32();

	int vertexCount = reader.ReadInt32();

	vec3* positions = nullptr, * normals = nullptr;
	vec4* weights = nullptr;
	vec2* texcoords = nullptr;

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
	{
		texcoords = (vec2*)BumpAllocatorMalloc(&memory->transientAllocator, vertexCount * sizeof(vec2));
		reader.ReadBytes(texcoords, vertexCount * sizeof(vec2));
	}

	if (hasVertexColors)
		reader.Skip(vertexCount * sizeof(uint32_t));

	if (hasBones)
	{
		weights = (vec4*)BumpAllocatorMalloc(&memory->transientAllocator, vertexCount * 2 * sizeof(vec4));
		reader.ReadBytes(weights, vertexCount * 2 * sizeof(vec4));
	}

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

	mesh->materialID = reader.ReadInt32();
	mesh->skeletonID = reader.ReadInt32();

	reader.Read(&mesh->boundingBox);
	reader.Read(&mesh->boundingSphere);

	InitMesh(mesh, vertexCount, positions, normals, weights, texcoords, indexCount, indices, indexElementSize, cmdBuffer);

	if (cacheData)
	{
		mesh->cachedPositions = positions;
		mesh->cachedIndices = indices;
	}
}

static Texture* ReadTexture(BinaryReader& reader, const char* scenePath, SDL_GPUCommandBuffer* cmdBuffer)
{
	char path[256];
	reader.ReadBytes(path, sizeof(path));

	int isEmbedded = reader.ReadInt32();

	if (isEmbedded)
	{
		int width = reader.ReadInt32();
		int height = reader.ReadInt32();

		if (width && height)
		{
			uint8_t* data = reader.CurrentPtr();
			uint32_t size = width * height * sizeof(uint32_t);
			reader.Skip(size);

			TextureInfo info = {};
			info.width = width;
			info.height = height;
			info.depth = 1;
			info.numMips = GetNumMipsForTexture(info.width, info.height);
			info.numLayers = 1;
			info.numFaces = 1;
			info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

			Texture* texture = LoadTextureFromData(data, size, &info, cmdBuffer);

			return texture;
		}
		else if (width > 0 && height == 0)
		{
			uint8_t* data = reader.CurrentPtr();
			reader.Skip(width);

			TextureInfo info = {};
			int channels;
			uint8_t* textureData = stbi_load_from_memory(data, width, &info.width, &info.height, &channels, 4);
			uint32_t size = info.width * info.height * 4;

			info.depth = 1;
			info.numMips = GetNumMipsForTexture(info.width, info.height);
			info.numLayers = 1;
			info.numFaces = 1;
			info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

			Texture* texture = LoadTextureFromData(textureData, size, &info, cmdBuffer);

			stbi_image_free(textureData);

			return texture;
		}
		else
		{
			SDL_assert(false);
			return nullptr;
		}
	}
	else
	{
		char absolutePath[256];
		GetAbsolutePath(absolutePath, 256, path, scenePath);
		SDL_strlcat(absolutePath, ".bin", 256);

		return LoadTexture(absolutePath, cmdBuffer);
	}
}

static void ReadMaterial(Material* material, BinaryReader& reader, const char* scenePath, SDL_GPUCommandBuffer* cmdBuffer)
{
	material->color = reader.ReadUInt32();
	material->metallicFactor = reader.ReadFloat();
	material->roughnessFactor = reader.ReadFloat();
	material->emissiveColor = reader.ReadVector3();
	material->emissiveStrength = reader.ReadFloat();

	int hasDiffuse = reader.ReadInt32();
	int hasNormal = reader.ReadInt32();
	int hasRoughness = reader.ReadInt32();
	int hasMetallic = reader.ReadInt32();
	int hasEmissive = reader.ReadInt32();
	int hasHeight = reader.ReadInt32();

	if (hasDiffuse) material->diffuse = ReadTexture(reader, scenePath, cmdBuffer);
	if (hasNormal) material->normal = ReadTexture(reader, scenePath, cmdBuffer);
	if (hasRoughness) material->roughness = ReadTexture(reader, scenePath, cmdBuffer);
	if (hasMetallic) material->metallic = ReadTexture(reader, scenePath, cmdBuffer);
	if (hasEmissive) material->emissive = ReadTexture(reader, scenePath, cmdBuffer);
	if (hasHeight) material->height = ReadTexture(reader, scenePath, cmdBuffer);
}

static void ReadSkeleton(Skeleton* skeleton, BinaryReader& reader)
{
	skeleton->numBones = reader.ReadInt32();

	for (int i = 0; i < skeleton->numBones; i++)
	{
		reader.ReadBytes(skeleton->bones[i].name, sizeof(skeleton->bones[i].name));

		skeleton->bones[i].offsetMatrix = reader.ReadMatrix4();
		skeleton->bones[i].nodeID = reader.ReadInt32();
	}

	skeleton->inverseBindPose = mat4::Identity;
}

static void ReadAnimation(Animation* animation, BinaryReader& reader)
{
	reader.ReadBytes(animation->name, sizeof(animation->name));

	animation->duration = reader.ReadFloat();

	animation->numPositions = reader.ReadInt32();
	animation->numRotations = reader.ReadInt32();
	animation->numScalings = reader.ReadInt32();
	animation->numChannels = reader.ReadInt32();

	animation->positions = (PositionKeyframe*)BumpAllocatorMalloc(&resource->animationCache.positionAllocator, animation->numPositions * sizeof(PositionKeyframe));
	animation->rotations = (RotationKeyframe*)BumpAllocatorMalloc(&resource->animationCache.rotationAllocator, animation->numRotations * sizeof(RotationKeyframe));
	animation->scalings = (ScalingKeyframe*)BumpAllocatorMalloc(&resource->animationCache.scalingAllocator, animation->numScalings * sizeof(ScalingKeyframe));

	SDL_assert(animation->positions);
	SDL_assert(animation->rotations);
	SDL_assert(animation->scalings);

	reader.ReadBytes(animation->positions, animation->numPositions * sizeof(PositionKeyframe));
	reader.ReadBytes(animation->rotations, animation->numRotations * sizeof(RotationKeyframe));
	reader.ReadBytes(animation->scalings, animation->numScalings * sizeof(ScalingKeyframe));

	SDL_assert(animation->numChannels <= MAX_ANIMATION_CHANNELS);

	for (int i = 0; i < animation->numChannels; i++)
	{
		reader.ReadBytes(animation->channels[i].name, sizeof(animation->channels[i].name));

		animation->channels[i].positionsOffset = reader.ReadInt32();
		animation->channels[i].positionsCount = reader.ReadInt32();
		animation->channels[i].rotationsOffset = reader.ReadInt32();
		animation->channels[i].rotationsCount = reader.ReadInt32();
		animation->channels[i].scalingsOffset = reader.ReadInt32();
		animation->channels[i].scalingsCount = reader.ReadInt32();
	}

	InitHashMap(&animation->channelNameMap);
	for (int i = 0; i < animation->numChannels; i++)
	{
		uint32_t nameHash = hash(animation->channels[i].name);
		HashMapAdd(&animation->channelNameMap, nameHash, i);
	}
}

static void ReadNode(Node* node, BinaryReader& reader)
{
	node->id = reader.ReadInt32();

	reader.ReadBytes(node->name, sizeof(node->name));

	node->armatureID = reader.ReadInt32();

	node->transform = reader.ReadMatrix4();

	node->numChildren = reader.ReadInt32();
	node->numMeshes = reader.ReadInt32();

	SDL_assert(node->numChildren <= MAX_NODE_CHILDREN);
	SDL_assert(node->numMeshes <= MAX_NODE_MESHES);

	reader.ReadBytes(node->children, node->numChildren * sizeof(int));
	reader.ReadBytes(node->meshes, node->numMeshes * sizeof(int));

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

bool LoadModel(Model* model, const char* path, bool cacheMeshes, SDL_GPUCommandBuffer* cmdBuffer)
{
	model->numMeshes = 0;

	// TODO free memory
	size_t fileSize;
	void* data = SDL_LoadFile(path, &fileSize);

	if (data)
	{
		BinaryReader reader((uint8_t*)data, (int)fileSize);

		model->numMeshes = reader.ReadInt32();
		model->numMaterials = reader.ReadInt32();
		model->numSkeletons = reader.ReadInt32();
		model->numAnimations = reader.ReadInt32();
		model->numNodes = reader.ReadInt32();
		int numLights = reader.ReadInt32();

		SDL_assert(model->numMeshes <= MAX_MESHES);
		SDL_assert(model->numMaterials <= MAX_MATERIALS);
		SDL_assert(model->numSkeletons <= MAX_SKELETONS);
		SDL_assert(model->numAnimations <= MAX_ANIMATIONS);
		SDL_assert(model->numNodes <= MAX_NODES);

		for (int i = 0; i < model->numMeshes; i++)
			ReadMesh(&model->meshes[i], reader, cacheMeshes, cmdBuffer);

		for (int i = 0; i < model->numMaterials; i++)
			ReadMaterial(&model->materials[i], reader, path, cmdBuffer);

		for (int i = 0; i < model->numSkeletons; i++)
			ReadSkeleton(&model->skeletons[i], reader);

		for (int i = 0; i < model->numAnimations; i++)
			ReadAnimation(&model->animations[i], reader);

		for (int i = 0; i < model->numNodes; i++)
			ReadNode(&model->nodes[i], reader);

		for (int i = 0; i < numLights; i++)
			ReadLight(reader);

		reader.Read(&model->boundingBox);
		reader.Read(&model->boundingSphere);

		SDL_free(data);

		return true;
	}

	return false;
}

Node* GetNodeByName(Model* model, const char* name)
{
	for (int i = 0; i < model->numNodes; i++)
	{
		if (SDL_strcmp(model->nodes[i].name, name) == 0)
			return &model->nodes[i];
	}
	return nullptr;
}

Animation* GetAnimationByName(Model* model, const char* name)
{
	for (int i = 0; i < model->numAnimations; i++)
	{
		if (SDL_strcmp(model->animations[i].name, name) == 0)
			return &model->animations[i];
	}
	return nullptr;
}
