#pragma once

#include "math/Vector.h"

#include <PxPhysics.h>
#include <PxScene.h>
#include <cooking/PxCooking.h>
#include <characterkinematic/PxControllerManager.h>
#include <cudamanager/PxCudaContextManager.h>
#include <pvd/PxPvd.h>


struct PhysicsAllocator : physx::PxAllocatorCallback
{
	void* allocate(size_t size, const char* typeName, const char* filename, int line) override;
	void deallocate(void* ptr) override;
};

struct PhysicsErrorCallback : physx::PxErrorCallback
{
	void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
};

struct RigidBody;
struct Mesh;

struct PhysicsState
{
	physx::PxFoundation* foundation;
	PhysicsAllocator allocator;
	PhysicsErrorCallback errorCallback;

	physx::PxPvd* pvd;

	physx::PxPhysics* physics;
	physx::PxScene* scene;
	physx::PxCookingParams cookingParams;
	physx::PxControllerManager* controllers;
	physx::PxMaterial* material;

	float timeAcc;
	bool running;
};

struct PhysicsHit
{
	float distance;
	vec3 position;
	vec3 normal;
	bool trigger;
	RigidBody* body;
};


bool InitPhysics(PhysicsState* physics);
void DestroyPhysics(PhysicsState* physics);

void StartPhysicsFrame(PhysicsState* physics);
void EndPhysicsFrame(PhysicsState* physics);

int Raycast(const vec3& origin, const vec3& direction, float distance, PhysicsHit* hits, int maxHits, uint32_t filterMask);
bool Raycast(const vec3& origin, const vec3& direction, float distance, uint32_t filterMask);
bool Linecast(vec3 point0, vec3 point1, uint32_t filterMask);
int OverlapBox(vec3 position, vec3 size, PhysicsHit* hits, int maxHits, uint32_t filterMask);
bool OverlapBox(vec3 position, vec3 size, uint32_t filterMask);
int OverlapSphere(const vec3& position, float radius, PhysicsHit* hits, int maxHits, uint32_t filterMask);
bool OverlapSphere(vec3 position, float radius, uint32_t filterMask);
int SweepSphere(float radius, const vec3& position, const vec3& direction, float maxDistance, PhysicsHit* hits, int maxHits, uint32_t filterMask);
bool SweepSphere(float radius, const vec3& position, const vec3& direction, float maxDistance, uint32_t filterMask);
