#version 460

#include "../common.glsl"

layout (location = 0) in vec3 v_normal;
layout (location = 1) in vec3 v_color;
layout (location = 2) in float v_roughness;

layout (location = 0) out vec4 out_normal;
layout (location = 1) out vec3 out_color;
layout (location = 2) out vec4 out_material;
layout (location = 3) out vec4 out_emissive;


layout(set = 2, binding = 0) uniform sampler2D s_diffuse;
layout(set = 2, binding = 1) uniform sampler2D s_roughness;
layout(set = 2, binding = 2) uniform sampler2D s_metallic;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 materialData0;
	vec4 materialData1;
	vec4 materialData2;
	vec4 materialData3;

#define hasDiffuse materialData0.x
#define hasRoughness materialData0.y
#define hasMetallic materialData0.z

#define materialColor materialData1.rgb
#define emissiveColor materialData2.rgb
#define emissiveStrength materialData2.a
#define roughnessFactor materialData3.r
#define metallicFactor materialData3.g
};


void main()
{
	float roughness = 1; //mix(roughnessFactor, texture(s_roughness, v_texcoord).g, hasRoughness);
	float metallic = 0; //mix(metallicFactor, texture(s_metallic, v_texcoord).b, hasMetallic);

	// dont need this because we're flipping in view space in the vertex shader
	vec3 normal = gl_FrontFacing ? v_normal : -v_normal;

	out_normal = vec4(normal * 0.5 + 0.5, emissiveStrength);
	out_color = v_color;
	out_material = vec4(v_roughness, metallic, 0, 0);
	out_emissive = vec4(linearToSRGB(emissiveColor), 0);
}
