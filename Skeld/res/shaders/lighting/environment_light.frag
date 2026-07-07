#version 460

#include "../common.glsl"
#include "lighting.glsl"

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_normal;
layout(set = 2, binding = 1) uniform sampler2D s_color;
layout(set = 2, binding = 2) uniform sampler2D s_material;
layout(set = 2, binding = 3) uniform sampler2D s_emissive;
layout(set = 2, binding = 4) uniform sampler2D s_depth;

layout(set = 2, binding = 5) uniform samplerCube s_environment;

layout(set = 3, binding = 0) uniform UniformBlock {
	mat4 projectionViewInv;
	mat4 viewInv;
	vec4 params;

#define cameraPosition params.xyz
};


vec3 sampleEnvironmentIrradiance(vec3 normal, samplerCube environmentMap)
{
	return textureLod(environmentMap, normal * vec3(1, 1, -1), log2(textureSize(environmentMap, 0).x)).rgb;
}

vec3 sampleEnvironmentPrefiltered(vec3 normal, vec3 view, float roughness, samplerCube environmentMap)
{
	vec3 r = reflect(-view, normal);
	float lodFactor = roughness; //1.0 - exp(-roughness * 12);

	return textureLod(environmentMap, r * vec3(1, 1, -1), lodFactor * log2(textureSize(environmentMap, 0).x)).rgb;
}

vec3 environmentLight(vec3 normal, vec3 view, vec3 albedo, float roughness, float metallic, samplerCube environmentMap)
{
	vec3 irradiance = sampleEnvironmentIrradiance(normal, environmentMap);

	vec3 diffuse = irradiance * albedo;

	vec3 f0 = mix(vec3(0.04), albedo, metallic);
	vec3 ks = fresnel2(max(dot(normal, view), 0.0), f0, roughness);
	vec3 kd = (1.0 - ks) * (1.0 - metallic);

	vec3 prefiltered = sampleEnvironmentPrefiltered(normal, view, roughness, environmentMap);

	vec2 brdfInteg = vec2(1.0, 0.0);
	vec3 specular = prefiltered * (ks * brdfInteg.r + brdfInteg.g);

	vec3 ambient = kd * diffuse + specular;

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
	float depth = texture(s_depth, v_texcoord).r;
	if (depth == 0)
		discard;

	vec3 position = reconstructPosition(v_texcoord, depth); // world space position
	vec3 view = normalize(cameraPosition - position); // world space view

	vec4 normalEmissiveStrength = texture(s_normal, v_texcoord);
	vec3 viewSpaceNormal = normalEmissiveStrength.rgb * 2 - 1;
	vec3 normal = (viewInv * vec4(viewSpaceNormal, 0)).xyz; // world space normal
	vec3 albedo = SRGBToLinear(texture(s_color, v_texcoord).rgb);

	vec4 material = texture(s_material, v_texcoord);
	float roughness = material.r;
	float metallic = material.g;

	vec3 radiance = environmentLight(normal, view, albedo, roughness, metallic, s_environment);

	vec3 emissiveColor = SRGBToLinear(texture(s_emissive, v_texcoord).rgb);
	float emissiveStrength = normalEmissiveStrength.a;
	radiance += emissiveColor * emissiveStrength;

	out_color = vec4(radiance, 1);
}
