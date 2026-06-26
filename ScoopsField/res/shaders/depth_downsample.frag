#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout (set = 2, binding = 0) uniform sampler2D s_depth;
layout (set = 2, binding = 1) uniform sampler2D s_normal;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;

#define size params.xy
#define mip int(params.z + 0.5)
};


void swap(inout vec4 val1, inout vec4 val2)
{
	vec4 tmp = val1;
	val1 = val2;
	val2 = tmp;
}

void main()
{
	vec2 texel = 1.0 / size;

	vec2 coord0 = v_texcoord - 0.25 * texel;
	vec2 coord1 = v_texcoord + 0.25 * vec2(texel.x, -texel.y);
	vec2 coord2 = v_texcoord + 0.25 * vec2(-texel.x, texel.y);
	vec2 coord3 = v_texcoord + 0.25 * texel;

	float depth0 = textureLod(s_depth, coord0, mip - 1).r;
	float depth1 = textureLod(s_depth, coord1, mip - 1).r;
	float depth2 = textureLod(s_depth, coord2, mip - 1).r;
	float depth3 = textureLod(s_depth, coord3, mip - 1).r;

	vec3 normal0 = textureLod(s_normal, coord0, mip - 1).rgb;
	vec3 normal1 = textureLod(s_normal, coord1, mip - 1).rgb;
	vec3 normal2 = textureLod(s_normal, coord2, mip - 1).rgb;
	vec3 normal3 = textureLod(s_normal, coord3, mip - 1).rgb;

	vec4 sample0 = vec4(normal0, depth0);
	vec4 sample1 = vec4(normal1, depth1);
	vec4 sample2 = vec4(normal2, depth2);
	vec4 sample3 = vec4(normal3, depth3);

	for (int i = 0; i < 3; i++)
	{
		if (sample0.w < sample1.w)
			swap(sample0, sample1);
		if (sample1.w < sample2.w)
			swap(sample1, sample2);
		if (sample2.w < sample3.w)
			swap(sample2, sample3);
	}

	//float depthThreshhold = 0.1;
	//float depthDiff = sample3.w - sample0.w;
    //vec4 result = depthDiff <= depthThreshhold ? 0.5 * sample1 + 0.5 * sample2 : sample1;

	vec4 result = sample1; //depth1 != 0 && depth2 != 0 ? 0.5 * sample1 + 0.5 * sample2 : sample1;

	out_color = vec4(result.rgb, 0);
	gl_FragDepth = result.w;
}
