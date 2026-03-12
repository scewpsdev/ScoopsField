#include "Animation.h"

#include "math/Math.h"


void InitAnimationState(AnimationState* animationState, Model* model)
{
	for (int i = 0; i < model->numNodes; i++)
	{
		animationState->nodeTransforms[i] = model->nodes[i].transform;
	}
	for (int i = 0; i < model->numMeshes; i++)
	{
		SkeletonState* skeletonState = &animationState->skeletons[model->meshes[i].skeletonID];
		skeletonState->numBones = model->skeletons[model->meshes[i].skeletonID].numBones;
		for (int j = 0; j < skeletonState->numBones; j++)
		{
			skeletonState->boneTransforms[j] = mat4::Identity;
		}
	}

	InitHashMap(&animationState->channelMap);
}

static int GetAnimationChannelWithName(Animation* animation, const char* name)
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

static mat4 AnimateNode(Node* node, AnimationChannel* channel, Animation* animation, float time, bool loop)
{
	if (loop)
		time = SDL_fmodf(time, animation->duration);

	vec3 position = AnimatePosition(&animation->positions[channel->positionsOffset], channel->positionsCount, time, animation->duration, loop);
	quat rotation = AnimateRotation(&animation->rotations[channel->rotationsOffset], channel->rotationsCount, time, animation->duration, loop);
	vec3 scaling = AnimateScaling(&animation->scalings[channel->scalingsOffset], channel->scalingsCount, time, animation->duration, loop);

	return mat4::Transform(position, rotation, scaling);
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

void AnimateModel(Model* model, AnimationState* animationState, Animation* animation, float time, bool loop, AnimationChannelFilterCallback_t channelFilter, void* filterUserPtr)
{
	SDL_assert(model->numNodes > 0);

	ClearHashMap(&animationState->channelMap);
	for (int i = 0; i < model->numNodes; i++)
	{
		Node* node = &model->nodes[i];
		if (channelFilter && !channelFilter(node, filterUserPtr))
			continue;
		int channelID = GetAnimationChannelWithName(animation, node->name);
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
			animationState->nodeTransforms[node->id] = AnimateNode(node, &animation->channels[channelID], animation, time, loop);
		}
	}
}

static mat4 interpolate(const mat4& a, const mat4& b, float blend)
{
	vec3 pa = a.translation();
	vec3 pb = b.translation();
	quat ra = a.rotation();
	quat rb = b.rotation();

	vec3 p = mix(pa, pb, blend);
	quat r = slerp(ra, rb, blend);

	return mat4::Translate(p) * mat4::Rotate(r);
}

void BlendAnimation(Model* model, AnimationState* animationState, Animation* animation, float time, bool loop, float blend, AnimationChannelFilterCallback_t channelFilter, void* filterUserPtr)
{
	SDL_assert(model->numNodes > 0);

	ClearHashMap(&animationState->channelMap);
	for (int i = 0; i < model->numNodes; i++)
	{
		Node* node = &model->nodes[i];
		if (channelFilter && !channelFilter(node, filterUserPtr))
			continue;
		int channelID = GetAnimationChannelWithName(animation, node->name);
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
			const mat4& b = AnimateNode(node, &animation->channels[channelID], animation, time, loop);
			animationState->nodeTransforms[node->id] = interpolate(a, b, blend);
		}
	}
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

void ApplyAnimationToSkeleton(Model* model, AnimationState* animationState)
{
	// resolve childOf constraints

	CalculateWorldTransform(0, {}, model, animationState);

	for (int i = 0; i < model->numMeshes; i++)
	{
		Mesh* mesh = &model->meshes[i];
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

mat4& GetNodeTransform(AnimationState* animationState, Node* node)
{
	return animationState->nodeTransforms[node->id];
}

void InitAnimation(AnimationPlayback* animation, const char* name, Model* moveset, float speed, bool loop, bool mirror)
{
	SDL_strlcpy(animation->name, name, 32);
	animation->speed = speed;
	animation->loop = loop;
	animation->mirror = mirror;
	animation->animation = GetAnimationByName(moveset, name);
}
