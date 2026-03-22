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
    mat4 projection;
	vec4 u_viewTexel;
};


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
}

void main()
{
	vec2 uv = gl_FragCoord.xy * u_viewTexel.xy;
	float depth = texture(s_depth, uv).r;
	if (depth == 1)
		discard;

	vec3 position = reconstructPosition(uv, depth);
	vec3 view = normalize(-position);

	vec3 normal = texture(s_normal, uv).rgb;
	vec3 albedo = texture(s_color, uv).rgb;

	vec4 material = texture(s_material, uv);
	float roughness = material.r;
	float metallic = material.g;

	vec3 radiance = pointLight(position, normal, view, albedo, roughness, metallic, v_lightPosition, v_lightColor);
	
	out_color = vec4(radiance, 1);
}
