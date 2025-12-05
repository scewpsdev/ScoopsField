#version 460

#include "common.shader"

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_hdrFrame;


void main()
{
	vec3 color = texture(s_hdrFrame, v_texcoord).rgb;
	color = linearToSRGB(color);

	out_color = vec4(color, 1);
}
