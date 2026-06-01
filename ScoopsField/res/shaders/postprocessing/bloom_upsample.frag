#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

#include "../common.glsl"


#define KERNEL_SIZE 3


layout(set = 2, binding = 0) uniform sampler2D s_input0;
layout(set = 2, binding = 1) uniform sampler2D s_input1;


void main()
{
	int x0 = -(KERNEL_SIZE - 1) / 2;
	int x1 = x0 + KERNEL_SIZE - 1;
	int y0 = -(KERNEL_SIZE - 1) / 2;
	int y1 = y0 + KERNEL_SIZE - 1;

	vec3 result = vec3(0);
	for (int y = y0; y <= y1; y++)
	{
		for (int x = x0; x <= x1; x++)
		{
			vec2 offset = vec2(x, y) / textureSize(s_input0, 0);
			vec3 value = texture(s_input0, v_texcoord + offset).rgb;
			result += value;
		}
	}

	vec3 input0 = result / (KERNEL_SIZE * KERNEL_SIZE);
	vec3 input1 = texture(s_input1, v_texcoord).rgb;

	out_color = vec4(input0 + input1, 1);
}
