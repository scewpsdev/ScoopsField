#version 460

#include "common.shader"

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_sky;
layout(set = 2, binding = 1) uniform sampler2D s_depth;
layout(set = 2, binding = 2) uniform sampler2D s_depthfullres;


void processSample(vec2 uv, inout vec4 color, inout float sum)
{
	float depth = texture(s_depth, uv).r;
	if (depth == 0)
	{
		color += texture(s_sky, uv);
		sum++;
	}
}

void main()
{
	vec2 texel = 1.0 / textureSize(s_sky, 0);

	float depth = texture(s_depthfullres, v_texcoord).r;
	if (depth != 0)
		discard;

	vec4 color = vec4(0);
	float sum = 0;

	processSample(v_texcoord, color, sum);
	processSample(v_texcoord + texel, color, sum);
	processSample(v_texcoord - texel, color, sum);
	processSample(v_texcoord + vec2(texel.x, -texel.y), color, sum);
	processSample(v_texcoord + vec2(-texel.x, texel.y), color, sum);
	processSample(v_texcoord + vec2(texel.x, 0), color, sum);
	processSample(v_texcoord + vec2(-texel.x, 0), color, sum);
	processSample(v_texcoord + vec2(0, texel.y), color, sum);
	processSample(v_texcoord + vec2(0, -texel.y), color, sum);

	color /= sum;
	out_color = vec4(color.rgb, 1);
}
