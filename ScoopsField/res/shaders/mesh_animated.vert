#version 460

#include "common.shader"

#define MAX_BONES 64

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec4 a_weights;
layout (location = 3) in vec4 a_indices;

layout (location = 0) out vec3 v_normal;
layout (location = 1) out vec3 v_color;


layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
	mat4 u_model;
	mat4 u_boneTransforms[MAX_BONES];
};


void main()
{
	vec4 weights = a_weights / (a_weights.x + a_weights.y + a_weights.z + a_weights.w);
	ivec4 indices = ivec4(a_indices + 0.5);

	mat4 boneTransform = mat4(
		vec4(0.0, 0.0, 0.0, 0.0),
		vec4(0.0, 0.0, 0.0, 0.0),
		vec4(0.0, 0.0, 0.0, 0.0),
		vec4(0.0, 0.0, 0.0, 0.0)
	);
	boneTransform += u_boneTransforms[indices.x] * weights.x;
	boneTransform += u_boneTransforms[indices.y] * weights.y;
	boneTransform += u_boneTransforms[indices.z] * weights.z;
	boneTransform += u_boneTransforms[indices.w] * weights.w;

	vec4 animatedPosition = boneTransform * vec4(a_position, 1);
	gl_Position = u_projectionViewModel * animatedPosition;

	vec4 animatedNormal = boneTransform * vec4(a_normal, 0);
	vec4 worldNormal = u_model * animatedNormal;

	v_normal = worldNormal.xyz;
	v_color = SRGBToLinear(vec3(0.5));
}
