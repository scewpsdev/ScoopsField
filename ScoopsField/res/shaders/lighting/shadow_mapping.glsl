


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

float calculateShadow(vec3 position, sampler2DShadow shadowMap, mat4 toLightSpace)
{
	const float SHADOW_MAP_EPSILON = -0.0001;

	vec4 lightSpacePosition = toLightSpace * vec4(position, 1);
	vec3 projectedCoords = lightSpacePosition.xyz / lightSpacePosition.w;
	vec2 sampleCoords = 0.5 * projectedCoords.xy * vec2(1, -1) + 0.5;

	//if (sampleCoords.x < 0.0 || sampleCoords.x > 1.0 || sampleCoords.y < 0.0 || sampleCoords.y > 1.0)
	//	return 1.0;

	ivec2 shadowMapSize = textureSize(shadowMap, 0);

	float result = 0.0;
	for (int i = 0; i < 16; i++)
	{
		//int idx = int(hash2(fragCoord.xy) + i) % 16;
		//vec2 poissonValue = stratifiedPoisson(idx);
		vec2 sampleStride = 1.0 / shadowMapSize;
		vec2 sampleOffset = stratifiedPoisson(i) * sampleStride;
		//vec4 shadowSample = textureGather(shadowMap, sampleCoords.xy + sampleOffset, 0);
		result += texture(shadowMap, vec3(sampleCoords.xy + sampleOffset, projectedCoords.z - SHADOW_MAP_EPSILON));
		//result += texture2D(shadowMap, sampleCoords + sampleOffset).r <= projectedCoords.z - SHADOW_MAP_EPSILON ? 0.0 : 1.0;
	}
	result /= 16;

	//float fadeOut = clamp(remap(distance / shadowMapFar, 0.9, 1.0, 1.0, 0.0), 0.0, 1.0);
	//fadeOut = 1.0 - ((1.0 - fadeOut) * fadeOutFactor);
	//result = 1.0 - ((1.0 - result) * fadeOut);

	return result;
}
