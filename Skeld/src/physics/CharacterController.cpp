#include "CharacterController.h"

#include "Physics.h"

#include "game/player/Player.h"

#include <characterkinematic/PxCapsuleController.h>

using namespace physx;


extern PhysicsState* physics;
extern float deltaTime;


static PxVec3 PxVector(const vec3& v)
{
	return PxVec3(v.x, v.y, v.z);
}

static PxVec3d PxVectord(const vec3& v)
{
	return PxVec3d(v.x, v.y, v.z);
}

static vec3 FromPxVector(const PxVec3& v)
{
	return vec3(v.x, v.y, v.z);
}

static vec3 FromPxVectord(const PxVec3d& v)
{
	return vec3((float)v.x, (float)v.y, (float)v.z);
}

class ControllerHitCallback : public physx::PxUserControllerHitReport
{
public:
	CharacterController* controller;

public:
	void onShapeHit(const physx::PxControllerShapeHit& hit) override;
	void onControllerHit(const physx::PxControllersHit& hit) override;
	void onObstacleHit(const physx::PxControllerObstacleHit& hit) override;
};

void ControllerHitCallback::onShapeHit(const PxControllerShapeHit& hit)
{
	vec3 position = FromPxVectord(hit.worldPos);
	vec3 normal = FromPxVector(hit.worldNormal);
	float length = hit.length;
	vec3 direction = FromPxVector(hit.dir);

	Player* player = (Player*)controller->controller->getUserData();
	OnControllerHit(player, position, normal, length, direction);
}

void ControllerHitCallback::onControllerHit(const PxControllersHit& hit)
{
}

void ControllerHitCallback::onObstacleHit(const PxControllerObstacleHit& hit)
{
}

void InitCharacterController(CharacterController* controller, float radius, float height, float stepOffset, const vec3& position, void* userPtr)
{
	PxCapsuleControllerDesc desc = {};
	desc.position = PxVectord(position + vec3::Up * (height * 0.5f + desc.contactOffset));
	desc.material = physics->material;
	desc.radius = radius;
	desc.height = height - 2 * radius;
	desc.stepOffset = stepOffset;
	desc.climbingMode = PxCapsuleClimbingMode::eEASY;
	desc.nonWalkableMode = PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;

	new((ControllerHitCallback*)controller->hitCallback)ControllerHitCallback();
	desc.reportCallback = (PxUserControllerHitReport*)controller->hitCallback;
	((ControllerHitCallback*)controller->hitCallback)->controller = controller;

	PxCapsuleController* capsule = (PxCapsuleController*)physics->controllers->createController(desc);
	capsule->setFootPosition((PxVec3d)PxVectord(position));
	capsule->setUserData(userPtr);

	controller->controller = capsule;
}

void DestroyCharacterController(CharacterController* controller)
{
	controller->controller->release();
}

ControllerCollisionFlags MoveCharacterController(CharacterController* controller, const vec3& delta, uint32_t filterMask)
{
	PxFilterData filterData = {};
	filterData.word0 = filterMask;

	PxControllerFilters filters = {};
	filters.mFilterData = &filterData;

	PxControllerCollisionFlags collisionFlags = controller->controller->move(PxVector(delta), 0, deltaTime, filters);
	return (ControllerCollisionFlags)(uint8_t)collisionFlags;
}

vec3 GetCharacterControllerPosition(CharacterController* controller)
{
	PxExtendedVec3 position = controller->controller->getFootPosition();
	return FromPxVectord(position);
}

void SetCharacterControllerPosition(CharacterController* controller, const vec3& position)
{
	controller->controller->setFootPosition(PxVectord(position));
}

void ResizeCharacterController(CharacterController* controller, float height)
{
	controller->controller->resize(height - 2 * controller->controller->getRadius());
}
