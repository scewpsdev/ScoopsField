#include "Articulation.h"

#include "Physics.h"
#include "Application.h"

#include <PxPhysicsAPI.h>


using namespace physx;

extern PhysicsState* physics;


static PxVec3 PxVector(const vec3& v)
{
	return PxVec3(v.x, v.y, v.z);
}

static PxQuat PxQuaternion(const quat& q)
{
	return PxQuat(q.x, q.y, q.z, q.w);
}

static PxTransform PxTransf(mat4 transform)
{
	return PxTransform(PxVector(transform.translation()), PxQuaternion(transform.rotation()));
}

void InitArticulation(Articulation* articulation)
{
	articulation->articulation = physics->physics->createArticulationReducedCoordinate();
}

void DestroyArticulation(Articulation* articulation)
{
	if (articulation->articulation->getScene())
		physics->scene->removeArticulation(*articulation->articulation);
	articulation->articulation->release();
}

void SpawnArticulation(Articulation* articulation)
{
	articulation->articulation->setRootLinearVelocity({});
	articulation->articulation->setRootAngularVelocity({});
	physics->scene->addArticulation(*articulation->articulation);
}

void SetArticulationRootTransform(Articulation* articulation, vec3 position, quat rotation)
{
	PxTransform transform = PxTransform(PxVector(position), PxQuaternion(rotation));
	articulation->articulation->setRootGlobalPose(transform);
}

void SetArticulationRootVelocity(Articulation* articulation, vec3 linearVelocity, vec3 angularVelocity)
{
	articulation->articulation->setRootLinearVelocity(PxVector(linearVelocity));
	articulation->articulation->setRootAngularVelocity(PxVector(angularVelocity));
}
