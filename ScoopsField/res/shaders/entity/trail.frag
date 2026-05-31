#version 460

#include "../common.glsl"

layout (location = 0) in vec2 v_texcoord;
layout (location = 1) in vec4 v_color;

layout (location = 0) out vec4 out_color;


void main()
{
	out_color = v_color;
}
