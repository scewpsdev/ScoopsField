#version 460

#include "../common.glsl"

layout (location = 0) in vec2 v_texcoord;
layout (location = 1) in vec4 v_color;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_texture;


void main()
{
	vec4 textureColor = texture(s_texture, v_texcoord);

	out_color = textureColor * v_color;
}
