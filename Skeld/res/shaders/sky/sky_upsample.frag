#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_sky;
layout(set = 2, binding = 1) uniform sampler2D s_depth; // full res depth

layout(set = 3, binding = 0) uniform UniformBlock {
	mat4 projectionInv;
	mat4 viewInv;
};


#include "../common.glsl"


vec3 reconstructView(vec2 uv, mat4 projectionInv, mat4 viewInv)
{
	vec2 ndc = vec2(uv.x * 2 - 1, uv.y * -2 + 1);

	vec3 dir;
	dir.x = ndc.x * projectionInv[0][0];
	dir.y = ndc.y * projectionInv[1][1];
	dir.z = -1;

	dir = mat3(viewInv) * dir;
	dir = normalize(dir);

	return dir;
}

vec3 sampleSkyViewLUT(vec3 dir)
{
	float longitude = mod(atan(dir.x, dir.z) + 2 * pi, 2 * pi);
	float latitude = asin(dir.y);

	float u = longitude / pi * 0.5;
	float v = 0.5 + 0.5 * -sign(latitude) * sqrt(abs(latitude) / pi * 2);

	vec3 color = texture(s_sky, vec2(u, v)).rgb;
	color = SRGBToLinear(color);

	return color;
}

bool processSample(vec2 uv, vec2 texel, inout vec3 color, inout float sum)
{
	uv /= texel;
	uv = floor(uv) + 0.5;
	uv *= texel;

	vec4 c = texture(s_sky, uv);
	if (c.a == 0)
	{
		color += c.rgb;
		sum++;
		return true;
	}
	return false;
}

bool processAerialSample(vec2 uv, vec2 texel, inout vec4 color, inout float sum)
{
	uv /= texel;
	uv = floor(uv) + 0.5;
	uv *= texel;

	vec4 c = texture(s_sky, uv);
	if (c.a != 0)
	{
		color += c;
		sum++;
		return true;
	}
	return false;
}

void main()
{
	vec2 texel = 1.0 / textureSize(s_sky, 0);
	float depth = texture(s_depth, v_texcoord).r;

	if (depth == 0)
	{
		vec4 c = texture(s_sky, v_texcoord);

		if (c.a == 0)
		{
			out_color = vec4(c.rgb, 1);
		}
		else
		{
			vec3 color = vec3(0);
			float sum = 0;
			
			processSample(v_texcoord + texel, texel, color, sum);
			processSample(v_texcoord - texel, texel, color, sum);
			processSample(v_texcoord + vec2(texel.x, -texel.y), texel, color, sum);
			processSample(v_texcoord + vec2(-texel.x, texel.y), texel, color, sum);

			if (sum == 0)
			{
				processSample(v_texcoord + vec2(texel.x, 0), texel, color, sum);
				processSample(v_texcoord + vec2(-texel.x, 0), texel, color, sum);
				processSample(v_texcoord + vec2(0, texel.y), texel, color, sum);
				processSample(v_texcoord + vec2(0, -texel.y), texel, color, sum);
			}

			if (sum > 0)
				color /= sum;

			out_color = vec4(color, 1);
		}
	}
	else
	{
		discard;
		vec2 uv = v_texcoord;
		uv /= texel;
		uv = floor(uv) + 0.5;
		uv *= texel;
		vec4 c = texture(s_sky, uv);

		if (c.a != 0)
		{
			out_color = c;
		}
		else
		{
			vec4 color = vec4(0);
			float sum = 0;

			processAerialSample(v_texcoord + texel, texel, color, sum);
			processAerialSample(v_texcoord - texel, texel, color, sum);
			processAerialSample(v_texcoord + vec2(texel.x, -texel.y), texel, color, sum);
			processAerialSample(v_texcoord + vec2(-texel.x, texel.y), texel, color, sum);

			if (sum > 0)
				color /= sum;

			out_color = color;
		}
	}
}
