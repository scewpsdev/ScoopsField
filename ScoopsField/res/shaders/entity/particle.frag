#version 460

#include "../common.glsl"

layout (location = 0) in vec4 v_color;
layout (location = 1) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_texture;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;
	vec4 materialData0;
	vec4 data1;
	vec4 data2;
	vec4 data3;

#define hasDiffuse materialData0.x

#define cameraPosition params.xyz
};


void main()
{
	vec4 textureColor = mix(vec4(1), texture(s_texture, v_texcoord), hasDiffuse);
	
	out_color = v_color * textureColor;
}
