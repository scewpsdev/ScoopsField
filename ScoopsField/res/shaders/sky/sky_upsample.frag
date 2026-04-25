#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_sky;
layout(set = 2, binding = 1) uniform sampler2D s_depth; // full res depth


bool processSample(vec2 uv, inout vec3 color, inout float sum)
{
	vec4 c = texture(s_sky, uv);
	if (c.a == 0)
	{
		color += c.rgb;
		sum++;
		return true;
	}
	return false;
}

bool processAerialSample(vec2 uv, inout vec4 color, inout float sum)
{
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
	if (depth != 0)
	{
		vec4 color = vec4(0);
		float sum = 0;

		if (!processAerialSample(v_texcoord, color, sum))
		{
			processAerialSample(v_texcoord + texel, color, sum);
			processAerialSample(v_texcoord - texel, color, sum);
			processAerialSample(v_texcoord + vec2(texel.x, -texel.y), color, sum);
			processAerialSample(v_texcoord + vec2(-texel.x, texel.y), color, sum);
		}
		//processSample(v_texcoord + vec2(texel.x, 0), color, sum);
		//processSample(v_texcoord + vec2(-texel.x, 0), color, sum);
		//processSample(v_texcoord + vec2(0, texel.y), color, sum);
		//processSample(v_texcoord + vec2(0, -texel.y), color, sum);

		if (sum > 0)
		{
			color /= sum;
			out_color = color;
		}
	}
	else
	{
		vec3 color = vec3(0);
		float sum = 0;

		if (!processSample(v_texcoord, color, sum))
		{
			processSample(v_texcoord + texel, color, sum);
			processSample(v_texcoord - texel, color, sum);
			processSample(v_texcoord + vec2(texel.x, -texel.y), color, sum);
			processSample(v_texcoord + vec2(-texel.x, texel.y), color, sum);
		}
		//processSample(v_texcoord + vec2(texel.x, 0), color, sum);
		//processSample(v_texcoord + vec2(-texel.x, 0), color, sum);
		//processSample(v_texcoord + vec2(0, texel.y), color, sum);
		//processSample(v_texcoord + vec2(0, -texel.y), color, sum);

		if (sum > 0)
		{
			color /= sum;
			out_color = vec4(color, 1);
		}
	}
}
