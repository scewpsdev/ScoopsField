#version 460

#include "lighting.glsl"

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_normal;
layout(set = 2, binding = 1) uniform sampler2D s_color;
layout(set = 2, binding = 2) uniform sampler2D s_material;
layout(set = 2, binding = 3) uniform sampler2D s_depth;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 lightData0;
	vec4 lightData1;
	mat4 projectionInv;

#define lightDirection lightData0.xyz
#define lightColor lightData1.rgb
};


// reconstruct without matrix multiplication just using near plane and fov
vec3 reconstructPosition(vec2 uv, float depth)
{
	vec4 ndc = vec4(uv.x * 2 - 1, uv.y * -2 + 1, depth, 1);
	vec4 viewSpacePosition = projectionInv * ndc;
	return viewSpacePosition.xyz / viewSpacePosition.w;
}

void main()
{
	float depth = texture(s_depth, v_texcoord).r;
	if (depth == 1)
		discard;

	vec3 position = reconstructPosition(v_texcoord, depth);
	vec3 view = normalize(-position);

	vec3 normal = texture(s_normal, v_texcoord).rgb;
	vec3 albedo = texture(s_color, v_texcoord).rgb;

	vec4 material = texture(s_material, v_texcoord);
	float roughness = material.r;
	float metallic = material.g;

	vec3 radiance = directionalLight(normal, view, albedo, roughness, metallic, lightDirection, lightColor);

	out_color = vec4(radiance, 1);
}
