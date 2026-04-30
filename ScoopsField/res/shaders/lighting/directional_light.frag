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
	vec4 lightData1;
	mat4 projection;

#define sunDirection lightData0.xyz
#define gameTime lightData0.w
//#define sunColor lightData1.rgb
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
	float shadow = 1.0; // Shadow mapping

	vec3 s = (specular + fLambert * kd) * radiance * ndotwi * shadow;

	return s;
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
	vec2 snappedUv = uv / texel;
	snappedUv = floor(snappedUv) + 0.25;
	snappedUv *= texel;

	float shadowDepth = texture(s_depth, snappedUv).r;

	float epsilon = 0.01;
	float weight = shadowDepth > 0 && abs(shadowDepth - depth) < epsilon ? 1 : 0;
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

	if (sum > 0)
		shadow /= sum;

	return shadow;
}

void main()
{
	float depth = texture(s_depth, v_texcoord).r;
	if (depth == 0)
		discard;

	vec3 position = reconstructPosition(v_texcoord, depth);
	vec3 view = normalize(-position);

	vec3 normal = texture(s_normal, v_texcoord).rgb;
	vec3 albedo = texture(s_color, v_texcoord).rgb;

	vec4 material = texture(s_material, v_texcoord);
	float roughness = material.r;
	float metallic = material.g;

	vec3 sunColor = texture(s_sunColor, vec2(0)).rgb;

	vec3 radiance = directionalLight(normal, view, albedo, roughness, metallic, sunDirection, sunColor);

	radiance *= upsampleShadowBuffer(v_texcoord, depth);
		
	out_color = vec4(radiance, 1);
}
