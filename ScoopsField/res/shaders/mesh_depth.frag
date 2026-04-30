#version 460

#include "common.glsl"

layout (location = 0) in vec3 v_normal;
layout (location = 1) in vec2 v_texcoord;

layout(set = 2, binding = 0) uniform sampler2D s_diffuse;
layout(set = 2, binding = 1) uniform sampler2D s_roughness;
layout(set = 2, binding = 2) uniform sampler2D s_metallic;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 materialData0;
	vec4 materialData1;
};


void main()
{
}
