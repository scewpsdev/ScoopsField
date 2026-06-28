#include "Animation.h"

#include "Application.h"

#include "math/Math.h"


void InitAnimationState(AnimationState* animationState, Model* model)
{
	animationState->model = model;

	animationState->nodeTransforms = (mat4*)MeshMalloc(model->numNodes * sizeof(mat4));
	SDL_memset(animationState->nodeTransforms, 0, model->numNodes * sizeof(mat4));

	for (int i = 0; i < model->numNodes; i++)
	{
		animationState->nodeTransforms[i] = model->nodes[i].transform;
	}

	for (int i = 0; i < model->numMeshes; i++)
	{
		if (model->meshes[i].skeletonID != -1)
		{
			SkeletonState* skeletonState = &animationState->skeletons[model->meshes[i].skeletonID];
			skeletonState->numBones = model->skeletons[model->meshes[i].skeletonID].numBones;

			skeletonState->boneTransforms = (mat4*)MeshMalloc(skeletonState->numBones * sizeof(mat4));
			SDL_memset(skeletonState->boneTransforms, 0, skeletonState->numBones * sizeof(mat4));

			for (int j = 0; j < skeletonState->numBones; j++)
			{
				skeletonState->boneTransforms[j] = mat4::Identity;
			}
		}
	}

	InitHashMap(&animationState->channelMap);
}

//
// [X] hitmarker
// [X] enemy visibility
// [X] blocking
// [X] blood particles
// [X] block particles
// [X] magic particles
// [X] bow secondary arrow cancel
// [X] arrows stick to rigid bodies
// [X] trail lighting
// [X] fire sconces
// [X] reflection probe point lights
// [X] particle lighting
// [X] damage types
// [X] fix block sparks
// [X] elevator collider
// [X] turn ik
// [X] swing trail
// [ ] fix point light flicker
// [ ] elevator sound effects
// [ ] player leg ik
// [ ] second block animation
// [ ] block/parry sound variation
// [ ] knight parry stagger animation
// [ ] entity attack parryable flag
// [X] kings sword secondary attacks
// [X] kings sword secondary follow ups
// [ ] kings sword backstab
// [ ] longsword moveset
// [ ] staff cast follow up
// [ ] staff melee secondary
// [ ] armor covers only certain body parts
// [ ] different impact effect and sound for armored body parts
// [ ] different armor resistance to different damage types
// [ ] damaging armor piece enough will knock it off
// [ ] armor condition damage independent from damage type
// [ ] enemy pathfinding
// [ ] healthbar
// [ ] enemy healthbar
// [ ] healing potions
// [ ] armor
// [ ] inventory ui
// [ ] 3d preview
// [ ] simple test combat rooms
//

void DestroyAnimationState(AnimationState* animationState)
{
	MeshFree(animationState->nodeTransforms);
	for (int i = 0; i < animationState->model->numMeshes; i++)
	{
		if (animationState->model->meshes[i].skeletonID != -1)
		{
			SkeletonState* skeletonState = &animationState->skeletons[animationState->model->meshes[i].skeletonID];
			MeshFree(skeletonState->boneTransforms);
		}
	}
	animationState->model = nullptr;
}

int GetAnimationChannelWithName(Animation* animation, const char* name)
{
	uint32_t nameHash = hash(name);
	if (int* channelID = HashMapGet(&animation->channelNameMap, nameHash))
		return *channelID;
	return -1;
}

static vec3 AnimatePosition(PositionKeyframe* positions, int numPositions, float time, float duration, bool loop)
{
	if (numPositions == 1) return positions[0].value;

	for (int i = numPositions - 1; i >= 0; i--)
	{
		float keyframeTime = positions[i].time;

		if (time >= keyframeTime)
		{
			int nextKeyframeIdx = loop ? (i + 1) % numPositions : min(i + 1, numPositions - 1);
			PositionKeyframe& keyframe0 = positions[i];
			PositionKeyframe& keyframe1 = positions[nextKeyframeIdx];
			float time0 = keyframe0.time;
			float time1 = keyframe1.time >= keyframe0.time ? keyframe1.time : keyframe1.time + duration;
			float blend = clamp(keyframe0.time != keyframe1.time ? (time - time0) / (time1 - time0) : 0.0f, 0.0f, 1.0f);
			return mix(keyframe0.value, keyframe1.value, blend);
		}
	}

	if (loop)
	{
		PositionKeyframe& keyframe0 = positions[numPositions - 1];
		PositionKeyframe& keyframe1 = positions[0];
		float blend = (time - (keyframe0.time - duration)) / (keyframe1.time - (keyframe0.time - duration));
		return mix(keyframe0.value, keyframe1.value, blend);
	}
	else
	{
		return positions[0].value;
	}
}

static quat AnimateRotation(RotationKeyframe* rotations, int numRotations, float time, float duration, bool loop)
{
	if (numRotations == 1) return rotations[0].value;

	for (int i = numRotations - 1; i >= 0; i--)
	{
		float keyframeTime = rotations[i].time;

		if (time >= keyframeTime)
		{
			int nextKeyframeIdx = loop ? (i + 1) % numRotations : min(i + 1, numRotations - 1);
			RotationKeyframe& keyframe0 = rotations[i];
			RotationKeyframe& keyframe1 = rotations[nextKeyframeIdx];
			float time0 = keyframe0.time;
			float time1 = keyframe1.time >= keyframe0.time ? keyframe1.time : keyframe1.time + duration;
			float blend = clamp(keyframe0.time != keyframe1.time ? (time - time0) / (time1 - time0) : 0.0f, 0.0f, 1.0f);
			return slerp(keyframe0.value, keyframe1.value, blend);
		}
	}

	if (loop)
	{
		RotationKeyframe& keyframe0 = rotations[numRotations - 1];
		RotationKeyframe& keyframe1 = rotations[0];
		float blend = (time - (keyframe0.time - duration)) / (keyframe1.time - (keyframe0.time - duration));
		return slerp(keyframe0.value, keyframe1.value, blend);
	}
	else
	{
		return rotations[0].value;
	}
}

static vec3 AnimateScaling(ScalingKeyframe* scalings, int numScalings, float time, float duration, bool loop)
{
	if (numScalings == 1) return scalings[0].value;

	for (int i = numScalings - 1; i >= 0; i--)
	{
		float keyframeTime = scalings[i].time;

		if (time >= keyframeTime)
		{
			int nextKeyframeIdx = loop ? (i + 1) % numScalings : min(i + 1, numScalings - 1);
			ScalingKeyframe& keyframe0 = scalings[i];
			ScalingKeyframe& keyframe1 = scalings[nextKeyframeIdx];
			float time0 = keyframe0.time;
			float time1 = keyframe1.time >= keyframe0.time ? keyframe1.time : keyframe1.time + duration;
			float blend = clamp(keyframe0.time != keyframe1.time ? (time - time0) / (time1 - time0) : 0.0f, 0.0f, 1.0f);
			return mix(keyframe0.value, keyframe1.value, blend);
		}
	}

	if (loop)
	{
		ScalingKeyframe& keyframe0 = scalings[numScalings - 1];
		ScalingKeyframe& keyframe1 = scalings[0];
		float blend = (time - (keyframe0.time - duration)) / (keyframe1.time - (keyframe0.time - duration));
		return mix(keyframe0.value, keyframe1.value, blend);
	}
	else
	{
		return scalings[0].value;
	}
}

mat4 AnimateNode(int channelID, Animation* animation, float time, bool loop, bool mirror)
{
	if (loop)
		time = mod(time, animation->duration);

	AnimationChannel* channel = &animation->channels[channelID];

	vec3 position = AnimatePosition(&animation->positions[channel->positionsOffset], channel->positionsCount, time, animation->duration, loop);
	quat rotation = AnimateRotation(&animation->rotations[channel->rotationsOffset], channel->rotationsCount, time, animation->duration, loop);
	vec3 scaling = AnimateScaling(&animation->scalings[channel->scalingsOffset], channel->scalingsCount, time, animation->duration, loop);

	mat4 transform = mat4::Transform(position, rotation, scaling);

	if (mirror)
	{
		transform.m30 *= -1.0f;
		transform.m01 *= -1.0f;
		transform.m02 *= -1.0f;
		transform.m10 *= -1.0f;
		transform.m20 *= -1.0f;
	}

	return transform;
}

mat4 AnimateNode(int channelID, AnimationPlayback* animation)
{
	return AnimateNode(channelID, animation->animation, animation->timer, animation->loop, animation->mirror);
}

static int GetNodeForMesh(int meshID, Model* model)
{
	for (int i = 0; i < model->numNodes; i++)
	{
		for (int j = 0; j < model->nodes[i].numMeshes; j++)
		{
			if (model->nodes[i].meshes[j] == meshID)
				return i;
		}
	}
	return -1;
}

void AnimateModel(Model* model, AnimationState* animationState, Animation* animation, float time, bool loop, bool mirror, AnimationChannelFilterCallback_t channelFilter, void* filterUserPtr)
{
	SDL_assert(model->numNodes > 0);

	ClearHashMap(&animationState->channelMap);
	for (int i = 0; i < model->numNodes; i++)
	{
		Node* node = &model->nodes[i];
		if (channelFilter && !channelFilter(node, filterUserPtr))
			continue;

		int channelID = -1;

		int nameLen = (int)SDL_strlen(node->name);
		bool mirroredNode = nameLen >= 3
			&& (node->name[nameLen - 1] == 'l' || node->name[nameLen - 1] == 'L' || node->name[nameLen - 1] == 'r' || node->name[nameLen - 1] == 'R')
			&& (node->name[nameLen - 2] == '_' || node->name[nameLen - 2] == '.');

		if (mirror && mirroredNode)
		{
			char mirroredName[64];
			SDL_memcpy(mirroredName, node->name, sizeof(node->name));
			mirroredName[nameLen - 1] += SDL_tolower(mirroredName[nameLen - 1]) == 'l' ? 'r' - 'l' : 'l' - 'r';
			Node* mirroredNode = GetNodeByName(model, mirroredName);
			channelID = GetAnimationChannelWithName(animation, mirroredName);
		}
		else
		{
			channelID = GetAnimationChannelWithName(animation, node->name);
		}

		if (channelID != -1)
		{
			HashMapAdd(&animationState->channelMap, node, channelID);
		}
	}

	for (int i = 0; i < animationState->channelMap.capacity; i++)
	{
		auto* slot = &animationState->channelMap.slots[i];
		if (slot->state == SLOT_USED)
		{
			Node* node = slot->key;
			int channelID = slot->value;

			animationState->nodeTransforms[node->id] = AnimateNode(channelID, animation, time, loop, mirror);
		}
	}
}

void AnimateModel(Model* model, AnimationState* animationState, AnimationPlayback* animation, AnimationChannelFilterCallback_t channelFilter, void* filterUserPtr)
{
	AnimateModel(model, animationState, animation->animation, animation->timer, animation->loop, animation->mirror, channelFilter, filterUserPtr);
}

void BlendAnimation(Model* model, AnimationState* animationState, Animation* animation, float time, bool loop, bool mirror, float blend, AnimationChannelFilterCallback_t channelFilter, void* filterUserPtr)
{
	SDL_assert(model->numNodes > 0);

	ClearHashMap(&animationState->channelMap);
	for (int i = 0; i < model->numNodes; i++)
	{
		Node* node = &model->nodes[i];
		if (channelFilter && !channelFilter(node, filterUserPtr))
			continue;

		int channelID = -1;

		int nameLen = (int)SDL_strlen(node->name);
		bool mirroredNode = nameLen >= 3
			&& (node->name[nameLen - 1] == 'l' || node->name[nameLen - 1] == 'L' || node->name[nameLen - 1] == 'r' || node->name[nameLen - 1] == 'R')
			&& (node->name[nameLen - 2] == '_' || node->name[nameLen - 2] == '.');

		if (mirror && mirroredNode)
		{
			char mirroredName[64];
			SDL_memcpy(mirroredName, node->name, sizeof(node->name));
			mirroredName[nameLen - 1] += SDL_tolower(mirroredName[nameLen - 1]) == 'l' ? 'r' - 'l' : 'l' - 'r';
			Node* mirroredNode = GetNodeByName(model, mirroredName);
			channelID = GetAnimationChannelWithName(animation, mirroredName);
		}
		else
		{
			channelID = GetAnimationChannelWithName(animation, node->name);
		}

		if (channelID != -1)
		{
			HashMapAdd(&animationState->channelMap, node, channelID);
		}
	}

	for (int i = 0; i < animationState->channelMap.capacity; i++)
	{
		auto* slot = &animationState->channelMap.slots[i];
		if (slot->state == SLOT_USED)
		{
			Node* node = slot->key;
			int channelID = slot->value;
			const mat4& a = animationState->nodeTransforms[node->id];
			const mat4& b = AnimateNode(channelID, animation, time, loop, mirror);
			animationState->nodeTransforms[node->id] = interpolate(a, b, blend);
		}
	}
}

void BlendAnimation(Model* model, AnimationState* animationState, AnimationPlayback* animation, float blend, AnimationChannelFilterCallback_t channelFilter, void* filterUserPtr)
{
	BlendAnimation(model, animationState, animation->animation, animation->timer, animation->loop, animation->mirror, blend, channelFilter, filterUserPtr);
}

static void CalculateWorldTransform(int id, const mat4& parentTransform, Model* model, AnimationState* animationState)
{
	Node* node = &model->nodes[id];

	const mat4& localTransform = animationState->nodeTransforms[id];
	mat4 transform = id > 0 ? parentTransform * localTransform : localTransform;
	animationState->nodeTransforms[id] = transform;

	for (int i = 0; i < node->numChildren; i++)
	{
		CalculateWorldTransform(node->children[i], transform, model, animationState);
	}
}

void ApplyAnimationToSkeleton(Model* model, AnimationState* animationState, bool calculateWorldTransforms)
{
	// resolve childOf constraints

	if (calculateWorldTransforms)
		CalculateWorldTransform(0, {}, model, animationState);

	for (int i = 0; i < model->numMeshes; i++)
	{
		Mesh* mesh = &model->meshes[i];
		if (mesh->skeletonID != -1)
		{
			int nodeID = GetNodeForMesh(i, model);
			SDL_assert(nodeID != -1 && nodeID < MAX_NODES);
			mat4 inverseBindPose = animationState->nodeTransforms[nodeID];

			Skeleton* skeleton = &model->skeletons[mesh->skeletonID];
			for (int j = 0; j < skeleton->numBones; j++)
			{
				Bone* bone = &skeleton->bones[j];
				animationState->skeletons[mesh->skeletonID].boneTransforms[j] = skeleton->inverseBindPose * animationState->nodeTransforms[bone->nodeID] * bone->offsetMatrix;
			}
		}
	}
}

mat4& GetNodeTransform(AnimationState* animationState, Node* node)
{
	//SDL_assert(HashMapGet(&animationState->channelMap, node));
	//SDL_assert(GetNodeByName(animationState->model, node->name) == node);
	SDL_assert(&animationState->model->nodes[node->id] == node);
	return animationState->nodeTransforms[node->id];
}

mat4 CalculateNodeWorldTransform(AnimationState* animationState, Node* node)
{
	mat4 transform = GetNodeTransform(animationState, node);
	while (node->parent != -1)
	{
		Node* parentNode = &animationState->model->nodes[node->parent];
		transform = GetNodeTransform(animationState, parentNode) * transform;
		node = parentNode;
	}
	return transform;
}

mat4 CalculateNodeDefaultWorldTransform(Model* model, Node* node)
{
	mat4 transform = node->transform;
	while (node->parent != -1)
	{
		Node* parentNode = &model->nodes[node->parent];
		transform = parentNode->transform * transform;
		node = parentNode;
	}
	return transform;
}

void InitAnimation(AnimationPlayback* animation, const char* name, Model* moveset, float speed, bool loop, bool mirror)
{
	SDL_strlcpy(animation->name, name, 32);
	animation->speed = speed;
	animation->loop = loop;
	animation->mirror = mirror;
	animation->animation = GetAnimationByName(moveset, name);
	animation->timer = 0;
}
