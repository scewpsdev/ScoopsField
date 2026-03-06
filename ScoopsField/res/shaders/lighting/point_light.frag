#version 460

#include "lighting.glsl"

layout (location = 0) in vec3 v_lightPosition;
layout (location = 1) in vec3 v_lightColor;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_normal;
layout(set = 2, binding = 1) uniform sampler2D s_color;
layout(set = 2, binding = 2) uniform sampler2D s_material;
layout(set = 2, binding = 3) uniform sampler2D s_depth;

layout(std140, set = 3, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewInv;
	vec4 u_viewTexel;
	vec4 cameraData;

	#define cameraPosition cameraData.xyz
};


vec3 reconstructPosition(vec2 uv, float depth)
{
	vec4 ndc = vec4(uv.x * 2 - 1, uv.y * -2 + 1, depth, 1);
	vec4 worldPosition = u_projectionViewInv * ndc;
	return worldPosition.xyz / worldPosition.w;
}

float attenuate(float dist)
{
	return 1.0 / (1.0 + 1 * dist + 2 * dist * dist);
}

void main()
{
	vec2 uv = gl_FragCoord.xy * u_viewTexel.xy;
	float depth = texture(s_depth, uv).r;
	if (depth == 1)
		discard;

	vec3 position = reconstructPosition(uv, depth);
	vec3 view = normalize(cameraPosition - position);

	vec3 normal = texture(s_normal, uv).rgb;
	vec3 albedo = texture(s_color, uv).rgb;

	vec4 material = texture(s_material, uv);
	float roughness = material.r;
	float metallic = material.g;

	//vec3 toLight = v_lightPosition - position;
	//float dist = length(toLight);
	//toLight /= dist;
	//float d = max(dot(normal, toLight), 0);
	//float a = attenuate(dist);
	//vec3 diffuse = a * d  * v_lightColor * color + 0.0001 * material.r;

	vec3 radiance = pointLight(position, normal, view, albedo, roughness, metallic, v_lightPosition, v_lightColor);
	
	out_color = vec4(radiance, 1);
}
