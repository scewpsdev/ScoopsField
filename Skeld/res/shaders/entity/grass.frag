#version 460

#include "../common.glsl"

layout (location = 0) in vec3 v_normal;
layout (location = 1) in vec3 v_color;
layout (location = 2) in float v_roughness;
layout (location = 3) in vec3 v_terrainNormal;
layout (location = 4) in float v_heightPercent2;
layout (location = 5) in vec3 v_position;

layout (location = 0) out vec4 out_normal;
layout (location = 1) out vec3 out_color;
layout (location = 2) out vec4 out_material;
layout (location = 3) out vec4 out_emissive;


layout(set = 2, binding = 0) uniform sampler2D s_sunColor;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 params;
	vec4 sunDirection;
	vec4 pointLightPositions[4];
	vec4 pointLightColors[4];

#define cameraPosition params.xyz
};


void main()
{
	float roughness = 1; //mix(roughnessFactor, texture(s_roughness, v_texcoord).g, hasRoughness);
	float metallic = 0; //mix(metallicFactor, texture(s_metallic, v_texcoord).b, hasMetallic);

	// dont need this because we're flipping in view space in the vertex shader
	vec3 normal = gl_FrontFacing ? v_normal : -v_normal;

	normal = mix(normal, v_terrainNormal, 1 - v_heightPercent2);
	normal = mix(normal, v_terrainNormal, v_heightPercent2);
	normal = normalize(normal);

	out_normal = vec4(normal * 0.5 + 0.5, 0);
	out_color = linearToSRGB(v_color); //vec3(normal * 0.5 + 0.5);
	out_material = vec4(v_roughness, metallic, 1, 0);
	out_emissive = vec4(0);
}
