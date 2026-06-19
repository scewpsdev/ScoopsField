#pragma once

#include "Model.h"

#include "utils/HashMap.h"


struct AnimationPlayback
{
	char name[32] = "";
	float speed = 1.0f;
	bool loop = true;
	bool mirror = false;

	Animation* animation;

	float timer;
};

struct SkeletonState
{
	mat4* boneTransforms;
	int numBones;
};

struct AnimationState
{
	Model* model;
	mat4* nodeTransforms;
	SkeletonState skeletons[MAX_SKELETONS];

	HashMap<Node*, int, MAX_NODES> channelMap;
};

typedef bool(*AnimationChannelFilterCallback_t)(Node* node, void* userPtr);


void InitAnimationState(AnimationState* animationState, Model* model);
void DestroyAnimationState(AnimationState* animationState);

mat4 AnimateNode(int channelID, Animation* animation, float time, bool loop, bool mirror);
mat4 AnimateNode(int channelID, AnimationPlayback* animation);
void AnimateModel(Model* model, AnimationState* animationState, Animation* animation, float time, bool loop, bool mirror, AnimationChannelFilterCallback_t channelFilter, void* filterUserPtr);
void AnimateModel(Model* model, AnimationState* animationState, AnimationPlayback* animation, AnimationChannelFilterCallback_t channelFilter, void* filterUserPtr);
void BlendAnimation(Model* model, AnimationState* animationState, Animation* animation, float time, bool loop, bool mirror, float blend, AnimationChannelFilterCallback_t channelFilter = nullptr, void* filterUserPtr = nullptr);
void BlendAnimation(Model* model, AnimationState* animationState, AnimationPlayback* animation, float blend, AnimationChannelFilterCallback_t channelFilter, void* filterUserPtr);
void ApplyAnimationToSkeleton(Model* model, AnimationState* animationState, bool calculateWorldTransforms = true);

int GetAnimationChannelWithName(Animation* animation, const char* name);
mat4& GetNodeTransform(AnimationState* animationState, Node* node);
mat4 CalculateNodeWorldTransform(AnimationState* animationState, Node* node);
mat4 CalculateNodeDefaultWorldTransform(Model* model, Node* node);

void InitAnimation(AnimationPlayback* animation, const char* name, Model* moveset, float speed, bool loop, bool mirror);
