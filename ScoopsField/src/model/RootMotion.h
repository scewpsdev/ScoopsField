#pragma once

#include "math/Matrix.h"


struct AnimationState;
struct Node;


mat4 DoRootMotion(AnimationState* anim, Node* rootNode, mat4* lastRootNodeTransform);
