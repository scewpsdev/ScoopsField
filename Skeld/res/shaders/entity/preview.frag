#version 460

#include "../common.glsl"

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

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
	vec4 textureColor = texture(s_diffuse, v_texcoord);
	textureColor = mix(vec4(1), SRGBToLinear(textureColor), hasDiffuse);

	float roughness = mix(roughnessFactor, texture(s_roughness, v_texcoord).g, hasRoughness);
	float metallic = mix(metallicFactor, texture(s_metallic, v_texcoord).b, hasMetallic);

	out_color = vec4(1, 1, 1, 0.1);
}
