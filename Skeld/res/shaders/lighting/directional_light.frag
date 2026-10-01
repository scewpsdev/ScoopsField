#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_normal;
layout(set = 2, binding = 1) uniform sampler2D s_color;
layout(set = 2, binding = 2) uniform sampler2D s_material;
layout(set = 2, binding = 3) uniform sampler2D s_depth;
layout(set = 2, binding = 4) uniform sampler2D s_sunColor;
layout(set = 2, binding = 5) uniform sampler2D s_shadows;

#include "../common.glsl"
#include "lighting.glsl"

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 lightData0;
	mat4 projection;

#define sunDirection lightData0.xyz
};


// Directional light indirect specular lighting
vec3 directionalLight(vec3 normal, vec3 view, vec3 albedo, float roughness, float metallic, vec3 lightDirection, vec3 lightColor)
{
	vec3 F0 = mix(vec3(0.04), albedo, metallic);

	// Per light radiance
	vec3 N = normal;
	vec3 V = view;
	vec3 L = -lightDirection;
	vec3 H = normalize(V + L);

	float NdotV = max(dot(N, V), 0);
	float NdotL = max(dot(N, L), 0);

	// Cook-Torrance BRDF
	float D = distributionGGX(N, H, roughness);
	float G = geometrySmith(NdotV, NdotL, roughness);
	vec3 F = fresnelSchlick(max(dot(H, L), 0), F0);
	vec3 numerator = D * F * G;
	float denominator = 4 * NdotV * NdotL;
	vec3 specular = numerator / max(denominator, 0.0000001);

	// valve horizon fade
	float fadeV = clamp(dot(N, V) * 4, 0, 1);
	float fadeL = clamp(dot(N, L) * 4, 0, 1);
	specular *= fadeV * fadeL;

	vec3 kS = F;
	vec3 kD = (1 - kS) * (1 - metallic);

	vec3 lambert = albedo / PI;
	vec3 diffuse = kD * lambert;

	vec3 radiance = lightColor;
	vec3 lighting = (diffuse + specular) * radiance * NdotL;

	return lighting;
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

void getShadowSample(vec2 uv, float depth, vec2 texel, inout float shadow, inout float sum)
{
	float near = projection[3][2];

	vec2 snappedUv = uv / texel;
	snappedUv = floor(snappedUv) + 0.25;
	snappedUv *= texel;

	float dist = near / depth;

	float shadowDepth = texture(s_depth, snappedUv).r;
	float shadowDistance = near / shadowDepth;
	
	float epsilon = 0.1 * dist;
	float weight = shadowDepth > 0 && abs(shadowDistance - dist) < epsilon ? 1 : 0;
	shadow += texture(s_shadows, uv).r * weight;
	sum += weight;
}

float upsampleShadowBuffer(vec2 uv, float depth)
{
	vec2 texel = 1.0 / textureSize(s_shadows, 0);
	float shadow = 0;
	float sum = 0;

	getShadowSample(uv, depth, texel, shadow, sum);
	getShadowSample(uv + 0.5 * vec2(texel.x, 0), depth, texel, shadow, sum);
	getShadowSample(uv + 0.5 * vec2(-texel.x, 0), depth, texel, shadow, sum);
	getShadowSample(uv + 0.5 * vec2(0, texel.y), depth, texel, shadow, sum);
	getShadowSample(uv + 0.5 * vec2(0, -texel.y), depth, texel, shadow, sum);
	//getShadowSample(uv + 0.5 * texel, depth, texel, shadow, sum);
	//getShadowSample(uv - 0.5 * texel, depth, texel, shadow, sum);
	//getShadowSample(uv + 0.5 * vec2(-texel.x, texel.y), depth, texel, shadow, sum);
	//getShadowSample(uv + 0.5 * vec2(texel.x, -texel.y), depth, texel, shadow, sum);

	shadow = sum > 0 ? shadow / sum : 1;

	return shadow;
}

void main()
{
	float depth = texture(s_depth, v_texcoord).r;
	if (depth == 0)
		discard;

	vec3 position = reconstructPosition(v_texcoord, depth);
	vec3 view = normalize(-position);

	vec3 normal = texture(s_normal, v_texcoord).rgb * 2 - 1;
	vec3 albedo = SRGBToLinear(texture(s_color, v_texcoord).rgb);

	vec4 material = texture(s_material, v_texcoord);
	float roughness = material.r;
	float metallic = material.g;

	vec3 sunColor = texture(s_sunColor, vec2(0.5)).rgb;

	vec3 radiance = directionalLight(normal, view, albedo, roughness, metallic, sunDirection, sunColor);

	radiance *= upsampleShadowBuffer(v_texcoord, depth);
		
	out_color = vec4(radiance, 1);
}
