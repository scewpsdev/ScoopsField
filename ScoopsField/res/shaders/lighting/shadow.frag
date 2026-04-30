#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_depth;
layout(set = 2, binding = 1) uniform sampler2DShadow s_shadowMap0;
layout(set = 2, binding = 2) uniform sampler2DShadow s_shadowMap1;
layout(set = 2, binding = 3) uniform sampler2DShadow s_shadowMap2;

#include "../common.glsl"

layout(set = 3, binding = 0) uniform UniformBlock {
	mat4 projection;
	mat4 toLightSpace0;
	mat4 toLightSpace1;
	mat4 toLightSpace2;

#define sunDirection lightData0.xyz
#define gameTime lightData0.w
//#define sunColor lightData1.rgb
};


const vec2 poissonDisk[16] =
{
	vec2(-0.94201624, -0.39906216),
	vec2(0.94558609, -0.76890725),
	vec2(-0.094184101, -0.92938870),
	vec2(0.34495938, 0.29387760),
	vec2(-0.91588581, 0.45771432),
	vec2(-0.81544232, -0.87912464),
	vec2(-0.38277543, 0.27676845),
	vec2(0.97484398, 0.75648379),
	vec2(0.44323325, -0.97511554),
	vec2(0.53742981, -0.47373420),
	vec2(-0.26496911, -0.41893023),
	vec2(0.79197514, 0.19090188),
	vec2(-0.24188840, 0.99706507),
	vec2(-0.81409955, 0.91437590),
	vec2(0.19984126, 0.78641367),
	vec2(0.14383161, -0.14100790)
};


vec2 stratifiedPoisson(int hash)
{
	return poissonDisk[hash % 16];
}

float calculateShadow(vec3 position, int cascade, sampler2DShadow shadowMap, mat4 toLightSpace)
{
	vec4 lightSpacePosition = toLightSpace * vec4(position, 1);
	vec3 projectedCoords = lightSpacePosition.xyz / lightSpacePosition.w;
	vec2 sampleCoords = 0.5 * projectedCoords.xy * vec2(1, -1) + 0.5;

	//if (sampleCoords.x < 0.0 || sampleCoords.x > 1.0 || sampleCoords.y < 0.0 || sampleCoords.y > 1.0)
	//	return 1.0;

	ivec2 shadowMapSize = textureSize(shadowMap, 0);
	float epsilon = 0.001 * (3 - cascade);

	float result = 0.0;
	for (int i = 0; i < 16; i++)
	{
		//int idx = int(hash2(fragCoord.xy) + i) % 16;
		//vec2 poissonValue = stratifiedPoisson(idx);
		vec2 sampleStride = 1.0 / shadowMapSize;
		vec2 sampleOffset = stratifiedPoisson(i) * sampleStride;
		//vec4 shadowSample = textureGather(shadowMap, sampleCoords.xy + sampleOffset, 0);
		result += texture(shadowMap, vec3(sampleCoords.xy + sampleOffset, projectedCoords.z - epsilon));
		//result += texture2D(shadowMap, sampleCoords + sampleOffset).r <= projectedCoords.z - SHADOW_MAP_EPSILON ? 0.0 : 1.0;
	}
	result /= 16;

	//float fadeOut = clamp(remap(distance / shadowMapFar, 0.9, 1.0, 1.0, 0.0), 0.0, 1.0);
	//fadeOut = 1.0 - ((1.0 - fadeOut) * fadeOutFactor);
	//result = 1.0 - ((1.0 - result) * fadeOut);

	return result;
}

// reconstruct without matrix multiplication just using near plane and fov
vec3 reconstructPosition(vec2 uv, float depth)
{
	vec2 ndc = vec2(uv.x * 2 - 1, 1 - uv.y * 2);
	float near = projection[3][2];
	float x = projection[0][0];
	float y = projection[1][1];

	vec3 view;
	view.z = near / depth;
	view.x = ndc.x * view.z / x;
	view.y = ndc.y * view.z / y;
	view.z *= -1; // right handed coordinate system
	return view;
	//vec4 viewSpacePosition = projectionInv * ndc;
	//return viewSpacePosition.xyz / viewSpacePosition.w;
}

void main()
{
	vec2 texel = 1.0 / (textureSize(s_depth, 0) / 2);

	vec2 snappedUv = v_texcoord / texel;
	snappedUv = floor(snappedUv) + 0.25;
	snappedUv *= texel;

	float depth = texture(s_depth, snappedUv).r;
	if (depth == 0)
		discard;

	vec3 position = reconstructPosition(v_texcoord, depth);

	float shadow = 1;
	if (-position.z < 15)
		shadow = calculateShadow(position, 0, s_shadowMap0, toLightSpace0);
	else if (-position.z < 60)
		shadow = calculateShadow(position, 1, s_shadowMap1, toLightSpace1);
	else
		shadow = calculateShadow(position, 2, s_shadowMap2, toLightSpace2);
		
	out_color = vec4(shadow, 0, 0, 0);
}
