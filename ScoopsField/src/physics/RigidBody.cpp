#include "RigidBody.h"

#include "Physics.h"
#include "Application.h"

#include "model/Model.h"

#include <PxActor.h>
#include <PxRigidBody.h>
#include <PxRigidActor.h>
#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>
#include <PxPhysicsAPI.h>

#include <SDL3/SDL.h>

using namespace physx;


extern GameMemory* memory;
extern PhysicsState* physics;


static PxVec3 PxVector(const vec3& v)
{
	return PxVec3(v.x, v.y, v.z);
}

static PxQuat PxQuaternion(const quat& q)
{
	return PxQuat(q.x, q.y, q.z, q.w);
}

static vec3 FromPxVector(const PxVec3& v)
{
	return vec3(v.x, v.y, v.z);
}

static quat FromPxQuaternion(const PxQuat& q)
{
	return quat(q.x, q.y, q.z, q.w);
}

static PxRigidActor* CreateActor(RigidBodyType type, const vec3& position, const quat& rotation)
{
	PxTransform transform(PxVector(position), PxQuaternion(rotation));

	switch (type)
	{
	case RIGID_BODY_DYNAMIC:
	case RIGID_BODY_KINEMATIC:
	{
		PxRigidDynamic* actor = physics->physics->createRigidDynamic(transform);

		if (type == RIGID_BODY_KINEMATIC)
			actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

		physics->scene->addActor(*actor);

		return actor;
	}
	case RIGID_BODY_STATIC:
	{
		PxRigidStatic* actor = physics->physics->createRigidStatic(transform);
		physics->scene->addActor(*actor);

		return actor;
	}
	}

	SDL_assert(false);
	return nullptr;
}

static void AddShape(PxRigidActor* actor, const PxGeometry& geometry, uint32_t filterGroup, uint32_t filterMask, const vec3& position, const quat& rotation, bool dynamic, bool trigger)
{
	SDL_assert(!(dynamic && trigger));

	PxShape* shape = physics->physics->createShape(geometry, *physics->material, true);
	shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, !trigger);
	shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, trigger);
	shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);

	PxFilterData filterData = {};
	filterData.word0 = filterGroup;
	filterData.word1 = filterMask;
	shape->setSimulationFilterData(filterData);
	shape->setQueryFilterData(filterData);

	PxTransform relativeTransform(PxVector(position), PxQuaternion(rotation));
	if (geometry.getType() == PxGeometryType::eCAPSULE)
		relativeTransform.q = relativeTransform.q * PxQuat(PxHalfPi, PxVec3(0, 0, 1));
	shape->setLocalPose(relativeTransform);

	actor->attachShape(*shape);

	if (dynamic)
	{
		float density = 1;
		PxRigidBodyExt::updateMassAndInertia(*(PxRigidBody*)actor, density);
	}
}

void InitRigidBody(RigidBody* body, RigidBodyType type, const vec3& position, const quat& rotation)
{
	body->type = type;
	body->actor = CreateActor(type, position, rotation);
	body->actor->userData = body;
}

void DestroyRigidBody(RigidBody* body)
{
	if (body->actor)
	{
		if (PxScene* scene = body->actor->getScene())
			scene->removeActor(*body->actor);
		body->actor->release();
	}
}

void AddBoxCollider(RigidBody* body, const vec3& size, const vec3& position, const quat& rotation, uint32_t filterGroup, uint32_t filterMask, bool trigger)
{
	AddShape(body->actor, PxBoxGeometry(PxVector(size * 0.5f)), filterGroup, filterMask, position, rotation, body->type == RIGID_BODY_DYNAMIC, trigger);
}

void AddSphereCollider(RigidBody* body, float radius, const vec3& position, const quat& rotation, uint32_t filterGroup, uint32_t filterMask, bool trigger)
{
	AddShape(body->actor, PxSphereGeometry(radius), filterGroup, filterMask, position, rotation, body->type == RIGID_BODY_DYNAMIC, trigger);
}

void AddCapsuleCollider(RigidBody* body, float radius, float height, const vec3& position, const quat& rotation, uint32_t filterGroup, uint32_t filterMask, bool trigger)
{
	AddShape(body->actor, PxCapsuleGeometry(radius, 0.5f * height - radius), filterGroup, filterMask, position, rotation, body->type == RIGID_BODY_DYNAMIC, trigger);
}

void AddMeshCollider(RigidBody* body, PxTriangleMesh* mesh, const vec3& position, const quat& rotation, const vec3& scale, uint32_t filterGroup, uint32_t filterMask, bool trigger)
{
	AddShape(body->actor, PxTriangleMeshGeometry(mesh, PxMeshScale(PxVector(scale))), filterGroup, filterMask, position, rotation, body->type == RIGID_BODY_DYNAMIC, trigger);
}

static void AddModelColliderNode(RigidBody* body, Model* model, Node* node, mat4 parentTransform, uint32_t filterGroup, uint32_t filterMask, bool trigger)
{
	mat4 nodeGlobalTransform = parentTransform * node->transform;

	for (int i = 0; i < node->numMeshes; i++)
	{
		int meshID = node->meshes[i];
		Mesh* mesh = &model->meshes[meshID];
		AddMeshCollider(body, CookTriangleMeshCollider(mesh), nodeGlobalTransform.translation(), nodeGlobalTransform.rotation(), nodeGlobalTransform.scale(), filterGroup, filterMask, trigger);
	}

	for (int i = 0; i < node->numChildren; i++)
	{
		AddModelColliderNode(body, model, &model->nodes[node->children[i]], nodeGlobalTransform, filterGroup, filterMask, trigger);
	}
}

void AddModelCollider(RigidBody* body, Model* model, const vec3& position, const quat& rotation, const vec3& scale, uint32_t filterGroup, uint32_t filterMask, bool trigger)
{
	AddModelColliderNode(body, model, &model->nodes[0], mat4::Transform(position, rotation, scale), filterGroup, filterMask, trigger);
}

void AddConvexMeshCollider(RigidBody* body, PxConvexMesh* mesh, const vec3& position, const quat& rotation, const vec3& scale, uint32_t filterGroup, uint32_t filterMask, bool trigger)
{
	AddShape(body->actor, PxConvexMeshGeometry(mesh, PxMeshScale(PxVector(scale))), filterGroup, filterMask, position, rotation, body->type == RIGID_BODY_DYNAMIC, trigger);
}

void RemoveColliders(RigidBody* body)
{
	uint32_t numShapes = body->actor->getNbShapes();
	PxShape** shapeBuffer = (PxShape**)BumpAllocatorMalloc(&memory->transientAllocator, numShapes * sizeof(PxShape*));
	body->actor->getShapes(shapeBuffer, numShapes);
	for (uint32_t i = 0; i < numShapes; i++)
	{
		body->actor->detachShape(*shapeBuffer[i]);
	}
}

void GetRigidBodyTransform(RigidBody* body, vec3* position, quat* rotation)
{
	PxRigidBody* dynamic = body->actor->is<PxRigidBody>();
	SDL_assert(dynamic);

	PxTransform transform = dynamic->getGlobalPose();
	if (position)
		*position = FromPxVector(transform.p);
	if (rotation)
		*rotation = FromPxQuaternion(transform.q);
}

void SetRigidBodyTransform(RigidBody* body, const vec3& position, const quat& rotation)
{
	PxTransform transform(PxVector(position), PxQuaternion(rotation));

	if (body->type == RIGID_BODY_DYNAMIC)
	{
		PxRigidDynamic* dynamic = body->actor->is<PxRigidDynamic>();
		SDL_assert(dynamic);

		dynamic->setGlobalPose(transform);
	}
	else if (body->type == RIGID_BODY_KINEMATIC)
	{
		PxRigidDynamic* dynamic = body->actor->is<PxRigidDynamic>();
		SDL_assert(dynamic);

		dynamic->setKinematicTarget(transform);
	}
	else if (body->type == RIGID_BODY_STATIC)
	{
		PxRigidStatic* stat = body->actor->is<PxRigidStatic>();
		SDL_assert(stat);

		stat->setGlobalPose(transform);
	}
	else
	{
		SDL_assert(false);
	}
}

void GetRigidBodyVelocity(RigidBody* body, vec3* velocity, vec3* angularVelocity)
{
	PxRigidDynamic* dynamic = body->actor->is<PxRigidDynamic>();
	SDL_assert(dynamic);

	if (velocity)
		*velocity = FromPxVector(dynamic->getLinearVelocity());
	if (angularVelocity)
		*angularVelocity = FromPxVector(dynamic->getAngularVelocity());
}

void SetRigidBodyVelocity(RigidBody* body, const vec3& velocity, const vec3& angularVelocity)
{
	PxRigidDynamic* dynamic = body->actor->is<PxRigidDynamic>();
	SDL_assert(dynamic);

	dynamic->setLinearVelocity(PxVector(velocity));
	dynamic->setAngularVelocity(PxVector(angularVelocity));
}

void AddRigidBodyAcceleration(RigidBody* body, const vec3& acceleration)
{
	PxRigidDynamic* dynamic = body->actor->is<PxRigidDynamic>();
	SDL_assert(dynamic);

	dynamic->addForce(PxVector(acceleration), PxForceMode::eACCELERATION);
}

void SetRigidBodyEnabled(RigidBody* body, bool enabled)
{
	body->actor->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, !enabled);
}

void SetRigidBodyAxisLock(RigidBody* body, uint8_t lockFlags)
{
	PxRigidDynamic* dynamic = body->actor->is<PxRigidDynamic>();
	SDL_assert(dynamic);

	dynamic->setRigidDynamicLockFlags((PxRigidDynamicLockFlags)lockFlags);
}

void GetRigidBodyAABB(RigidBody* body, vec3* center, vec3* size)
{
	PxBounds3 bounds = body->actor->getWorldBounds();
	if (center)
		*center = FromPxVector(bounds.getCenter());
	if (size)
		*size = FromPxVector(bounds.getDimensions());
}

PxTriangleMesh* CookTriangleMeshCollider(Mesh* mesh)
{
	int numVertices = mesh->vertexCount;
	int vertexStride = sizeof(vec3);
	void* vertices = mesh->cachedPositions;

	int numIndices = mesh->indexCount;
	int indexStride = GetIndexFormatSize(mesh->indexBuffer->elementSize);
	void* indices = mesh->cachedIndices;

	PxTriangleMeshDesc meshDesc = {};
	meshDesc.points.count = numVertices;
	meshDesc.points.stride = vertexStride;
	meshDesc.points.data = vertices;

	meshDesc.triangles.count = numIndices / 3;
	meshDesc.triangles.stride = 3 * indexStride;
	meshDesc.triangles.data = indices;
	meshDesc.flags = PxMeshFlags(mesh->indexBuffer->elementSize == SDL_GPU_INDEXELEMENTSIZE_16BIT ? PxMeshFlag::e16_BIT_INDICES : 0);

	meshDesc.isValid();

	PxDefaultMemoryOutputStream writeBuffer;
	PxTriangleMeshCookingResult::Enum result;
	bool status = PxCookTriangleMesh(physics->cookingParams, meshDesc, writeBuffer, &result);
	if (!status)
	{
		printf("Failed to cook triangle mesh\n");
		return nullptr;
	}

	PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	PxTriangleMesh* meshCollider = physics->physics->createTriangleMesh(readBuffer);

	return meshCollider;
}
