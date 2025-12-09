#include "RigidBody.h"

#include "Physics.h"

#include <PxActor.h>
#include <PxRigidBody.h>
#include <PxRigidActor.h>
#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>
#include <PxPhysicsAPI.h>

#include <SDL3/SDL.h>

using namespace physx;


extern PhysicsState* physics;


static PxVec3 PxVector(const vec3& v)
{
	return PxVec3(v.x, v.y, v.z);
}

static PxQuat PxQuaternion(const Quaternion& q)
{
	return PxQuat(q.x, q.y, q.z, q.w);
}

static vec3 FromPxVector(const PxVec3& v)
{
	return vec3(v.x, v.y, v.z);
}

static Quaternion FromPxQuaternion(const PxQuat& q)
{
	return Quaternion(q.x, q.y, q.z, q.w);
}

static PxRigidActor* CreateActor(RigidBodyType type, const vec3& position, const Quaternion& rotation)
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

static void AddShape(PxRigidActor* actor, const PxGeometry& geometry, uint32_t filterGroup, uint32_t filterMask, const vec3& position, const Quaternion& rotation, bool dynamic, bool trigger)
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

void InitRigidBody(RigidBody* body, RigidBodyType type, const vec3& position, const Quaternion& rotation)
{
	body->type = type;
	body->actor = CreateActor(type, position, rotation);
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

void AddBoxCollider(RigidBody* body, const vec3& size, const vec3& position, const Quaternion& rotation, uint32_t filterGroup, uint32_t filterMask, bool trigger)
{
	AddShape(body->actor, PxBoxGeometry(PxVector(size * 0.5f)), filterGroup, filterMask, position, rotation, body->type == RIGID_BODY_DYNAMIC, trigger);
}

void AddSphereCollider(RigidBody* body, float radius, const vec3& position, const Quaternion& rotation, uint32_t filterGroup, uint32_t filterMask, bool trigger)
{
	AddShape(body->actor, PxSphereGeometry(radius), filterGroup, filterMask, position, rotation, body->type == RIGID_BODY_DYNAMIC, trigger);
}

void AddCapsuleCollider(RigidBody* body, float radius, float height, const vec3& position, const Quaternion& rotation, uint32_t filterGroup, uint32_t filterMask, bool trigger)
{
	AddShape(body->actor, PxCapsuleGeometry(radius, 0.5f * height - radius), filterGroup, filterMask, position, rotation, body->type == RIGID_BODY_DYNAMIC, trigger);
}

void AddMeshCollider(RigidBody* body, PxTriangleMesh* mesh, const vec3& position, const Quaternion& rotation, const vec3& scale, uint32_t filterGroup, uint32_t filterMask, bool trigger)
{
	AddShape(body->actor, PxTriangleMeshGeometry(mesh, PxMeshScale(PxVector(scale))), filterGroup, filterMask, position, rotation, body->type == RIGID_BODY_DYNAMIC, trigger);
}

void AddConvexMeshCollider(RigidBody* body, PxConvexMesh* mesh, const vec3& position, const Quaternion& rotation, const vec3& scale, uint32_t filterGroup, uint32_t filterMask, bool trigger)
{
	AddShape(body->actor, PxConvexMeshGeometry(mesh, PxMeshScale(PxVector(scale))), filterGroup, filterMask, position, rotation, body->type == RIGID_BODY_DYNAMIC, trigger);
}

void GetRigidBodyTransform(RigidBody* body, vec3* position, Quaternion* rotation)
{
	PxRigidBody* dynamic = body->actor->is<PxRigidBody>();
	SDL_assert(dynamic);

	PxTransform transform = dynamic->getGlobalPose();
	*position = FromPxVector(transform.p);
	*rotation = FromPxQuaternion(transform.q);
}

void SetRigidBodyTransform(RigidBody* body, const vec3& position, const Quaternion& rotation)
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

	*velocity = FromPxVector(dynamic->getLinearVelocity());
	*angularVelocity = FromPxVector(dynamic->getAngularVelocity());
}

void SetRigidBodyVelocity(RigidBody* body, const vec3& velocity, const vec3& angularVelocity)
{
	PxRigidDynamic* dynamic = body->actor->is<PxRigidDynamic>();
	SDL_assert(dynamic);

	dynamic->setLinearVelocity(PxVector(velocity));
	dynamic->setAngularVelocity(PxVector(angularVelocity));
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
	*center = FromPxVector(bounds.getCenter());
	*size = FromPxVector(bounds.getDimensions());
}
