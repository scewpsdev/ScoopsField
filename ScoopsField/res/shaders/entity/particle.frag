#version 460

#include "../common.glsl"

layout (location = 0) in vec4 v_color;

layout (location = 0) out vec4 out_color;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;
	vec4 data0;
	vec4 data1;
	vec4 data2;
	vec4 data3;

#define cameraPosition params.xyz
};


void main()
{
	out_color = v_color;
}
