#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_normal;
layout(set = 2, binding = 1) uniform sampler2D s_color;
layout(set = 2, binding = 2) uniform sampler2D s_material;
layout(set = 2, binding = 3) uniform sampler2D s_depth;
layout(set = 2, binding = 4) uniform sampler2D s_sunColor;
layout(set = 2, binding = 5) uniform sampler2DShadow s_shadowMap;
layout(set = 2, binding = 6) uniform samplerCube s_skybox;

#include "../common.glsl"
#include "lighting.glsl"

layout(set = 3, binding = 0) uniform UniformBlock {
	mat4 projectionViewInv;
	mat4 projectionInv;
	mat4 viewInv;
	mat4 toLightSpace;
	vec4 params;
	vec4 params2;

#define sunDirection params.xyz
#define cameraPosition params2.xyz
};


// Directional light indirect specular lighting
vec3 directionalLight(vec3 normal, vec3 view, vec3 albedo, float roughness, float metallic, vec3 lightDirection, vec3 lightColor)
{
	vec3 f0 = mix(vec3(0.04), albedo, metallic);
	vec3 fLambert = albedo / PI;

	// Per light radiance
	vec3 wi = -lightDirection;
	vec3 h = normalize(view + wi);

	// Cook-Torrance BRDF
	float d = normalDistribution(normal, h, roughness);
	float g = geometrySmith(normal, view, wi, roughness);
	vec3 f = fresnel2(max(dot(h, view), 0.0), f0, roughness);
	vec3 numerator = d * f * g;
	float denominator = 4.0 * max(dot(view, normal), 0.0) * max(dot(wi, normal), 0.0);
	vec3 specular = numerator / max(denominator, 0.0001);

	vec3 ks = f;
	vec3 kd = (1.0 - ks) * (1.0 - metallic);

	vec3 radiance = lightColor;

	float ndotwi = max(dot(wi, normal), 0.0);

	vec3 s = (specular + fLambert * kd) * radiance * ndotwi;

	return s;
}

vec3 reconstructPosition(vec2 uv, float depth)
{
	vec4 ndc = vec4(uv.x * 2 - 1, uv.y * -2 + 1, depth, 1);
	vec4 worldPosition = projectionViewInv * ndc;
	return worldPosition.xyz / worldPosition.w;
}

vec3 reconstructView(vec2 uv, mat4 projectionInv, mat4 viewInv)
{
	vec2 ndc = vec2(uv.x * 2 - 1, uv.y * -2 + 1);

	//float aspect = projection[1][1] / projection[0][0];
	//ndc.x *= aspect;

	//float tanHalfFov = 1.0 / projection[1][1]; //tan(fov * 0.5);

	vec3 dir;
	dir.x = ndc.x * projectionInv[0][0];
	dir.y = ndc.y * projectionInv[1][1];
	dir.z = -1;

	dir = mat3(viewInv) * dir;
	dir = normalize(dir);

	return dir;
}

float calculateShadow(vec3 position, vec3 normal, vec3 toLight, sampler2DShadow shadowMap, mat4 toLightSpace)
{
	vec4 lightSpacePosition = toLightSpace * vec4(position, 1);
	vec3 projectedCoords = lightSpacePosition.xyz / lightSpacePosition.w;
	vec2 sampleCoords = 0.5 * projectedCoords.xy * vec2(1, -1) + 0.5;

	//if (sampleCoords.x < 0.0 || sampleCoords.x > 1.0 || sampleCoords.y < 0.0 || sampleCoords.y > 1.0)
	//	return 1.0;

	ivec2 shadowMapSize = textureSize(shadowMap, 0);
	
	float shadowBias = 0.01; //0.00002 + 0.0001 * (3 - cascade);
	shadowBias += max(0.006 * (1 - dot(normal, toLight)), 0);

	float shadow = texture(shadowMap, vec3(sampleCoords.xy, projectedCoords.z - shadowBias));

	return shadow;
}

void main()
{
	float depth = texture(s_depth, v_texcoord).r;

	if (depth == 0)
	{
		vec3 view = reconstructView(v_texcoord, projectionInv, viewInv);
		out_color = vec4(texture(s_skybox, view).rgb, 1);
		return;
	}

	vec3 position = reconstructPosition(v_texcoord, depth);
	vec3 view = normalize(cameraPosition - position);

	vec3 viewSpaceNormal = texture(s_normal, v_texcoord).rgb * 2 - 1;
	vec3 normal = (viewInv * vec4(viewSpaceNormal, 0)).xyz; // world space normal
	vec3 albedo = texture(s_color, v_texcoord).rgb;

	vec4 material = texture(s_material, v_texcoord);
	float roughness = material.r;
	float metallic = material.g;

	vec3 sunColor = texture(s_sunColor, vec2(0.0)).rgb;

	vec3 radiance = directionalLight(normal, view, albedo, roughness, metallic, sunDirection, sunColor);

	radiance *= calculateShadow(position, normal, -sunDirection, s_shadowMap, toLightSpace);
		
	out_color = vec4(radiance, 1);
}
