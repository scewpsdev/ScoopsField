#include "Physics.h"

#include "Application.h"

#include <physx/PxPhysicsAPI.h>

#include <new>


using namespace physx;


#define PHYSICS_TICKS 60
#define MAX_PHYSICS_STEPS_PER_FRAME 10

#define InitTrashCppObject(x, T) new(x)T()


extern GameMemory* memory;
extern float deltaTime;


void* PhysicsAllocator::allocate(size_t size, const char* typeName, const char* filename, int line)
{
	memory->physicsMemoryUsage += size;
	memory->physicsAllocationCount++;
	memory->physicsAllocationsPerFrame++;
	void* mem = malloc(size);
	//if (HashMapHasSlot(&memory->physicsAllocations))
	HashMapAdd(&memory->physicsAllocations, mem, size);
	return mem;
}

void PhysicsAllocator::deallocate(void* ptr)
{
	free(ptr);
	uint64_t* memsize = HashMapRemove(&memory->physicsAllocations, ptr);
	memory->physicsMemoryUsage -= *memsize;
	memory->physicsAllocationCount--;
}

void PhysicsErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
{
	SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "PhysX Error %d at %s:%d: %s", (int)code, file, line, message);
}

static PxFilterFlags FilterShader(
	PxFilterObjectAttributes attributes0,
	PxFilterData filterData0,
	PxFilterObjectAttributes attributes1,
	PxFilterData filterData1,
	PxPairFlags& pairFlags,
	const void* constantBlock,
	PxU32 constantBlockSize)
{
	PX_UNUSED(constantBlock);
	PX_UNUSED(constantBlockSize);

	if ((filterData0.word0 & filterData1.word1) || (filterData1.word0 & filterData0.word1))
	{
		// let triggers through
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlag::eDEFAULT;
		}

		pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND;
		return PxFilterFlag::eDEFAULT;
	}
	return PxFilterFlag::eKILL;
}


bool InitPhysics(PhysicsState* physics)
{
	InitTrashCppObject(&physics->allocator, PhysicsAllocator);
	InitTrashCppObject(&physics->errorCallback, PhysicsErrorCallback);

	physics->foundation = PxCreateFoundation(PX_PHYSICS_VERSION, physics->allocator, physics->errorCallback);
	if (!physics->foundation)
	{
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to initialize PhysX foundation");
		return false;
	}

#if _DEBUG
	physics->pvd = PxCreatePvd(*physics->foundation);
	if (!physics->pvd)
	{
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to initialize PVD instance");
	}

	PxPvdTransport* pvdTransport = PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
	if (!physics->pvd->connect(*pvdTransport, PxPvdInstrumentationFlag::eALL))
	{
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to connect PVD instance");
	}
#endif

	PxTolerancesScale scale;
	bool trackAllocations = false;
	physics->physics = PxCreatePhysics(PX_PHYSICS_VERSION, *physics->foundation, scale, trackAllocations, physics->pvd);
	if (!physics->physics)
	{
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to initialize PhysX module");
		return false;
	}

	PxSceneDesc sceneDesc(scale);
	sceneDesc.gravity = PxVec3(0, -10, 0);
	sceneDesc.filterShader = FilterShader;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(8);
	if (!sceneDesc.cpuDispatcher)
	{
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to create PhysX CPU dispatcher");
		return false;
	}

	physics->scene = physics->physics->createScene(sceneDesc);
	if (!physics->scene)
	{
		SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to create PhysX scene");
		return false;
	}

	physics->controllers = PxCreateControllerManager(*physics->scene);

	physics->material = physics->physics->createMaterial(0.5f, 0.5f, 0.1f);

	physics->timeAcc = 0.0f;
	physics->running = false;

	return true;
}

void DestroyPhysics(PhysicsState* physics)
{
	physics->material->release();
	physics->controllers->release();
	physics->scene->release();
	physics->physics->release();
	if (physics->pvd)
		physics->pvd->release();
	physics->foundation->release();
}

void StartPhysicsFrame(PhysicsState* physics)
{
	float timeStep = 1.0f / PHYSICS_TICKS;
	physics->timeAcc += deltaTime;

	if (!physics->running)
	{
		// read transforms here

		int numSteps = (int)SDL_floorf(physics->timeAcc / timeStep);
		if (numSteps > 0)
		{
			physics->scene->simulate(timeStep);
			physics->timeAcc -= timeStep * numSteps;
			physics->running = true;
		}
	}
}

void EndPhysicsFrame(PhysicsState* physics)
{
	if (physics->running)
	{
		uint32_t error;
		physics->scene->fetchResults(true, &error);
		if (error)
		{
			SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "PhysX hardware error: %u", error);
		}

		// write transforms here

		physics->running = false;
	}
}
