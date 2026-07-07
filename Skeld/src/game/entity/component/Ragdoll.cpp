#include "Ragdoll.h"

#include "game/entity/Entity.h"

#include "renderer/Renderer.h"


static void InitRagdollNode(Ragdoll* ragdoll, Node* node, Node* parent, RigidBody* parentBody, mat4 parentTransform, Creature* creature)
{
	RigidBody* bone = nullptr;
	mat4 boneTransform = parentTransform * node->transform;

	uint32_t h = hash(node->name);
	if (RigidBody* hitbox = HashMapGet(&creature->hitboxes, h))
	{
		bone = HashMapAdd(&ragdoll->bones, h, {});
		InitRigidBody(bone, &ragdoll->articulation, parentBody, true, boneTransform, ragdoll);
		CopyColliders(bone, hitbox, ENTITY_FILTER_RAGDOLL, ENTITY_FILTER_DEFAULT);
	}

	for (int i = 0; i < node->numChildren; i++)
	{
		Node* child = &ragdoll->model->nodes[node->children[i]];
		InitRagdollNode(ragdoll, child, bone ? node : parent, bone ? bone : parentBody, boneTransform, creature);
	}
}

static void AnimateRagdollNode(Ragdoll* ragdoll, Node* node, RigidBody* parentHitbox, mat4 parentHitboxTransform, Creature* creature)
{
	RigidBody* hitbox = nullptr;
	mat4 hitboxTransform;

	uint32_t h = hash(node->name);
	if (hitbox = HashMapGet(&creature->hitboxes, h))
	{
		RigidBody* bone = HashMapGet(&ragdoll->bones, h);

		vec3 hitboxPosition;
		quat hitboxRotation;
		GetRigidBodyTransform(hitbox, &hitboxPosition, &hitboxRotation);

		hitboxTransform = mat4::Transform(hitboxPosition, hitboxRotation);

		vec3 hitboxVelocity;
		vec3 hitboxAngularVelocity;
		GetRigidBodyVelocity(hitbox, &hitboxVelocity, &hitboxAngularVelocity);

		if (parentHitbox)
		{
			mat4 localTransform = parentHitboxTransform.inverted() * hitboxTransform;
			mat4 localOffset = GetJointParentPose(bone).inverted() * localTransform;
			quat rotation = localOffset.rotation();
			vec3 eulers = rotation.eulers();

			SetJointRotation(bone, eulers);

			//SetJointVelocity(bone, hitboxAngularVelocity);
			//SetJointVelocity(bone, vec3(0));
		}
		else
		{
			SetArticulationRootTransform(&ragdoll->articulation, hitboxPosition, hitboxRotation);
			//SetArticulationRootVelocity(&ragdoll->articulation, hitboxVelocity, hitboxAngularVelocity);
			//SetArticulationRootVelocity(&ragdoll->articulation, vec3(0), vec3(0));
		}
	}

	for (int i = 0; i < node->numChildren; i++)
	{
		Node* child = &ragdoll->model->nodes[node->children[i]];
		AnimateRagdollNode(ragdoll, child, hitbox ? hitbox : parentHitbox, hitbox ? hitboxTransform : parentHitboxTransform, creature);
	}
}

void InitRagdoll(Ragdoll* ragdoll, Creature* creature)
{
	InitEntity((Entity*)ragdoll, ENTITY_TYPE_RAGDOLL);

	ragdoll->position = creature->position;
	ragdoll->rotation = creature->rotation;

	ragdoll->model = creature->model;
	InitAnimationState(&ragdoll->anim, ragdoll->model);

	SDL_assert(creature->hitboxes.numUsedSlots);
	InitHashMap(&ragdoll->bones);

	InitArticulation(&ragdoll->articulation);

	Node* root = &ragdoll->model->nodes[0];
	InitRagdollNode(ragdoll, root, nullptr, nullptr, mat4::Rotate(vec3::Up, PI), creature);
	AnimateRagdollNode(ragdoll, root, nullptr, mat4::Identity, creature);

	SpawnArticulation(&ragdoll->articulation);
}

void DestroyRagdoll(Ragdoll* ragdoll)
{
	DestroyArticulation(&ragdoll->articulation);

	/*
	for (int i = 0; i < ragdoll->bones.capacity; i++)
	{
		if (ragdoll->bones.slots[i].state == SLOT_USED)
		{
			RigidBody* bone = &ragdoll->bones.slots[i].value;
			DestroyRigidBody(bone);
		}
	}
	*/
}

RigidBody* GetRagdollBodyWithName(Ragdoll* ragdoll, const char* name)
{
	uint32_t h = hash(name);
	return HashMapGet(&ragdoll->bones, h);
}

static void UpdateRagdollNode(Ragdoll* ragdoll, Node* node, mat4 parentTransform, mat4 modelMatrixInv)
{
	mat4 transform;

	uint32_t h = hash(node->name);
	if (RigidBody* bone = HashMapGet(&ragdoll->bones, h))
	{
		vec3 bonePosition;
		quat boneRotation;
		GetRigidBodyTransform(bone, &bonePosition, &boneRotation);

		transform = mat4::Transform(bonePosition, boneRotation);
		transform = modelMatrixInv * transform;
	}
	else
	{
		transform = parentTransform * node->transform;
	}

	GetNodeTransform(&ragdoll->anim, node) = transform;

	for (int i = 0; i < node->numChildren; i++)
	{
		Node* child = &ragdoll->model->nodes[node->children[i]];
		UpdateRagdollNode(ragdoll, child, transform, modelMatrixInv);
	}
}

void UpdateRagdoll(Ragdoll* ragdoll)
{
	Node* root = &ragdoll->model->nodes[0];
	UpdateRagdollNode(ragdoll, root, mat4::Identity, ModelMatrix((Entity*)ragdoll).inverted());

	ApplyAnimationToSkeleton(ragdoll->model, &ragdoll->anim, false);
}

void RenderRagdoll(Ragdoll* ragdoll)
{
	RenderModel(&game->renderer, ragdoll->model, &ragdoll->anim, ModelMatrix((Entity*)ragdoll));
}
