#include "Model.h"
#include "Animation.h"


mat4 DoRootMotion(AnimationState* anim, Node* rootNode, mat4* lastRootNodeTransform, Animation* animation, Animation* lastAnimation)
{
	mat4& rootNodeTransform = GetNodeTransform(anim, rootNode);
	mat4 delta = animation == lastAnimation ? rootNodeTransform * lastRootNodeTransform->inverted() : mat4::Identity;
	*lastRootNodeTransform = rootNodeTransform;
	rootNodeTransform = mat4::Identity;
	return delta;
}
