#pragma once

#include "Model.h"

#include "utils/HashMap.h"


struct SkeletonState
{
	mat4 boneTransforms[MAX_BONES];
	int numBones;
};

struct AnimationState
{
	mat4 nodeTransforms[MAX_NODES];
	SkeletonState skeletons[MAX_SKELETONS];

	HashMap<Node*, int, MAX_NODES> channelMap;
};

typedef bool(*AnimationChannelFilterCallback_t)(Node* node, void* userPtr);


void InitAnimationState(AnimationState* animationState, Model* model);

void AnimateModel(Model* model, AnimationState* animationState, Animation* animation, float time, bool loop);
void BlendAnimation(Model* model, AnimationState* animationState, Animation* animation, float time, bool loop, float blend, AnimationChannelFilterCallback_t channelFilter = nullptr, void* filterUserPtr = nullptr);
void ApplyAnimationToSkeleton(Model* model, AnimationState* animationState);

const mat4& GetNodeTransform(AnimationState* animationState, Node* node);
