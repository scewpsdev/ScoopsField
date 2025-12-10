#include "Animation.h"

#include "math/Math.h"


void InitAnimationState(AnimationState* animationState, Model* model)
{
	for (int i = 0; i < model->numNodes; i++)
	{
		animationState->nodeTransforms[i] = mat4::Identity;
	}
	for (int i = 0; i < model->numMeshes; i++)
	{
		animationState->skeletons[i].numBones = model->skeletons[model->meshes[i].skeletonID].numBones;
		for (int j = 0; j < animationState->skeletons[i].numBones; j++)
		{
			animationState->skeletons[i].boneTransforms[j] = mat4::Identity;
		}
	}
}

static int GetAnimationChannelWithName(Animation* animation, const char* name)
{
	for (int i = 0; i < animation->numChannels; i++)
	{
		if (SDL_strcmp(animation->channels[i].name, name) == 0)
			return i;
	}
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

static void AnimateNode(int id, mat4 parentTransform, Model* model, AnimationState* animationState, Animation* animation, float time, bool loop)
{
	Node* node = &model->nodes[id];
	mat4 localTransform = node->transform;

	int channelID = GetAnimationChannelWithName(animation, node->name);
	if (channelID != -1)
	{
		AnimationChannel* channel = &animation->channels[channelID];

		vec3 position = AnimatePosition(&animation->positions[channel->positionsOffset], channel->positionsCount, SDL_fmodf(time, animation->duration), animation->duration, loop);
		quat rotation = AnimateRotation(&animation->rotations[channel->rotationsOffset], channel->rotationsCount, SDL_fmodf(time, animation->duration), animation->duration, loop);
		vec3 scaling = AnimateScaling(&animation->scalings[channel->scalingsOffset], channel->scalingsCount, SDL_fmodf(time, animation->duration), animation->duration, loop);

		localTransform = mat4::Transform(position, rotation, scaling);
	}

	mat4 transform = id > 0 ? parentTransform * localTransform : localTransform;
	animationState->nodeTransforms[id] = transform;

	for (int i = 0; i < node->numChildren; i++)
	{
		AnimateNode(node->children[i], animationState->nodeTransforms[id], model, animationState, animation, time, loop);
	}
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

void AnimateModel(Model* model, AnimationState* animationState, Animation* animation, float time)
{
	SDL_assert(model->numNodes > 0);
	AnimateNode(0, {}, model, animationState, animation, time, true);

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
			animationState->skeletons[i].boneTransforms[j] = skeleton->inverseBindPose * animationState->nodeTransforms[bone->nodeID] * bone->offsetMatrix;
		}
	}
}
