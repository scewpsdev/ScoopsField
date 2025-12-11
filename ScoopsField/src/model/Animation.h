#pragma once

#include "Model.h"


struct SkeletonState
{
	mat4 boneTransforms[MAX_BONES];
	int numBones;
};

struct AnimationState
{
	mat4 nodeTransforms[MAX_NODES];
	SkeletonState skeletons[MAX_MESHES];
};


void InitAnimationState(AnimationState* animationState, Model* model);

void AnimateModel(Model* model, AnimationState* animationState, Animation* animation, float time, bool loop);
