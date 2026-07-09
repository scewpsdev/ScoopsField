#version 460

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_normal;
layout(set = 2, binding = 1) uniform sampler2D s_color;
layout(set = 2, binding = 2) uniform sampler2D s_material;
layout(set = 2, binding = 3) uniform sampler2D s_depth;
layout(set = 2, binding = 4) uniform samplerCube s_cubemap;
layout(set = 2, binding = 5) uniform sampler2D s_brdf;

layout(set = 2, binding = 6) readonly buffer SHBuffer {
	vec3 coefficients[9];
};

layout(std140, set = 3, binding = 0) uniform UniformBlock {
	mat4 projectionViewInv;
	mat4 viewInv;
	vec4 viewTexel;
	vec4 params;
	vec4 params2;
	vec4 params3;

#define probePosition_ params.xyz
#define probeSize_ params2.xyz
#define cameraPosition params3.xyz
};

#include "../common.glsl"
#include "lighting.glsl"
#include "sh_reconstruct.glsl"


vec3 sampleEnvironmentPrefiltered(vec3 position, vec3 normal, vec3 view, float roughness, samplerCube environmentMap, vec3 probePosition, vec3 probeSize)
{
	//float lodFactor = 1 - (1 - roughness) * (1 - roughness); //1.0 - exp(-roughness * 12);

	vec3 r = reflect(-view, normal);
	vec3 localPos = position - probePosition;
	vec3 toSample = parallaxCorrect(localPos, r, probeSize);
	vec3 dir = localPos + toSample;

	float maxLod = 5; //log2(textureSize(environmentMap, 0).x);
	float lod = roughness * maxLod;

	return textureLod(environmentMap, dir * vec3(1, 1, -1), lod).rgb;
}

vec3 environmentLight(vec3 position, vec3 normal, vec3 view, vec3 albedo, float roughness, float metallic, samplerCube environmentMap, vec3 probePosition, vec3 probeSize)
{
	vec3 irradiance = getIrradiance(position, normal, coefficients, probePosition, probeSize);

	vec3 diffuse = irradiance * albedo;

	vec3 f0 = mix(vec3(0.04), albedo, metallic);
	vec3 kS = fresnel2(max(dot(normal, view), 0.0), f0, roughness);
	vec3 kD = (1.0 - kS) * (1.0 - metallic);

	vec3 prefiltered = sampleEnvironmentPrefiltered(position, normal, view, roughness, environmentMap, probePosition, probeSize);

	vec2 brdf = texture(s_brdf, vec2(max(dot(normal, view), 0), roughness)).rg;
	vec3 specular = prefiltered * (kS * brdf.x + brdf.y);

	vec3 ambient = kD * diffuse + specular;

	return ambient;
}

vec3 reconstructPosition(vec2 uv, float depth)
{
	vec4 ndc = vec4(uv.x * 2 - 1, uv.y * -2 + 1, depth, 1);
	vec4 worldPosition = projectionViewInv * ndc;
	return worldPosition.xyz / worldPosition.w;
}

void main()
{
	vec2 uv = gl_FragCoord.xy * viewTexel.xy;
	float depth = texture(s_depth, uv).r;
	if (depth == 0)
		discard;

	vec3 position = reconstructPosition(uv, depth); // world space position
	vec3 view = normalize(cameraPosition - position); // world space view

	vec3 viewSpaceNormal = texture(s_normal, uv).rgb * 2 - 1;
	vec3 normal = normalize((viewInv * vec4(viewSpaceNormal, 0)).xyz); // world space normal
	vec3 albedo = SRGBToLinear(texture(s_color, uv).rgb);

	vec4 material = texture(s_material, uv);
	float roughness = material.r;
	float metallic = material.g;

	vec3 radiance = environmentLight(position, normal, view, albedo, roughness, metallic, s_cubemap, probePosition_, probeSize_);

	float sdf = length(max(abs(position - probePosition_) - probeSize_, 0));
	const float maxDistance = 1.0;
	float alpha = max(remap(sdf, 0, maxDistance, 1, 0), 0);

	out_color = vec4(radiance, alpha);
}
