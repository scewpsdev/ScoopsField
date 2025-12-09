#pragma once

#include <physx/PxPhysics.h>
#include <physx/PxScene.h>
#include <physx/characterkinematic/PxControllerManager.h>
#include <physx/pvd/PxPvd.h>


struct PhysicsAllocator : physx::PxAllocatorCallback
{
	void* allocate(size_t size, const char* typeName, const char* filename, int line) override;
	void deallocate(void* ptr) override;
};

struct PhysicsErrorCallback : physx::PxErrorCallback
{
	void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
};

struct PhysicsState
{
	physx::PxFoundation* foundation;
	PhysicsAllocator allocator;
	PhysicsErrorCallback errorCallback;

	physx::PxPvd* pvd;

	physx::PxPhysics* physics;
	physx::PxScene* scene;
	physx::PxControllerManager* controllers;
	physx::PxMaterial* material;

	float timeAcc;
	bool running;
};


bool InitPhysics(PhysicsState* physics);
void DestroyPhysics(PhysicsState* physics);

void StartPhysicsFrame(PhysicsState* physics);
void EndPhysicsFrame(PhysicsState* physics);
