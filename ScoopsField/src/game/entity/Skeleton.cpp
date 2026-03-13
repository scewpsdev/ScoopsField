#include "Skeleton.h"

#include "renderer/Renderer.h"

#include "game/Game.h"
#include "Resource.h"


void InitSkeleton(Entity* skeleton, const vec3& position, float rotation, int health)
{
	InitEntity(skeleton, ENTITY_TYPE_SKELETON);

	skeleton->position = position;

	skeleton->creature.hasValue = true;
	Creature* creature = &skeleton->creature.value;
	InitCreature(creature, skeleton, "entities/skeleton/skeleton.glb", rotation, health);
}
