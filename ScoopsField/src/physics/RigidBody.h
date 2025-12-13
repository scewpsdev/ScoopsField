#pragma once

#include "math/Vector.h"
#include "math/Quaternion.h"
#include "math/Matrix.h"

#include <PxPhysics.h>


enum RigidBodyAxisLock
{
	RIGID_BODY_LOCK_X = 1 << 0,
	RIGID_BODY_LOCK_Y = 1 << 1,
	RIGID_BODY_LOCK_Z = 1 << 2,
	RIGID_BODY_LOCK_ROT_X = 1 << 3,
	RIGID_BODY_LOCK_ROT_Y = 1 << 4,
	RIGID_BODY_LOCK_ROT_Z = 1 << 5,

	RIGID_BODY_LOCK_TRANSLATION = RIGID_BODY_LOCK_X | RIGID_BODY_LOCK_Y | RIGID_BODY_LOCK_Z,
	RIGID_BODY_LOCK_ROTATION = RIGID_BODY_LOCK_ROT_X | RIGID_BODY_LOCK_ROT_Y | RIGID_BODY_LOCK_ROT_Z,
};

enum RigidBodyType
{
	RIGID_BODY_DYNAMIC,
	RIGID_BODY_KINEMATIC,
	RIGID_BODY_STATIC,
};

struct RigidBody
{
	RigidBodyType type;
	physx::PxRigidActor* actor;

	void* userPtr;
};


void InitRigidBody(RigidBody* body, RigidBodyType bodyType, const vec3& position, const quat& rotation);
void DestroyRigidBody(RigidBody* body);

void AddBoxCollider(RigidBody* body, const vec3& size, const vec3& position, const quat& rotation, uint32_t filterGroup, uint32_t filterMask, bool trigger);
void AddSphereCollider(RigidBody* body, float radius, const vec3& position, const quat& rotation, uint32_t filterGroup, uint32_t filterMask, bool trigger);
void AddCapsuleCollider(RigidBody* body, float radius, float height, const vec3& position, const quat& rotation, uint32_t filterGroup, uint32_t filterMask, bool trigger);
void AddMeshCollider(RigidBody* body, physx::PxTriangleMesh* mesh, const vec3& position, const quat& rotation, const vec3& scale, uint32_t filterGroup, uint32_t filterMask, bool trigger);
void AddConvexMeshCollider(RigidBody* body, physx::PxConvexMesh* mesh, const vec3& position, const quat& rotation, const vec3& scale, uint32_t filterGroup, uint32_t filterMask, bool trigger);
void RemoveColliders(RigidBody* body);

void GetRigidBodyTransform(RigidBody* body, vec3* position, quat* rotation);
void SetRigidBodyTransform(RigidBody* body, const vec3& position, const quat& rotation);
void GetRigidBodyVelocity(RigidBody* body, vec3* velocity, vec3* angularVelocity);
void SetRigidBodyVelocity(RigidBody* body, const vec3& velocity, const vec3& angularVelocity);
void SetRigidBodyEnabled(RigidBody* body, bool enabled);
void SetRigidBodyAxisLock(RigidBody* body, uint8_t lockFlags);
void GetRigidBodyAABB(RigidBody* body, vec3* center, vec3* size);
