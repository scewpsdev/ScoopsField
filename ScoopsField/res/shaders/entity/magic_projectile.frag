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
	vec4 params;
	vec4 materialData0;
	vec4 materialData1;
	vec4 materialData2;
	vec4 data3;

#define hasDiffuse materialData0.x
#define hasRoughness materialData0.y
#define hasMetallic materialData0.z

#define materialColor materialData1.rgb
#define emissiveColor materialData2.rgb
#define emissiveStrength materialData2.a

#define cameraPosition params.xyz
};


void main()
{
	vec3 normal = normalize(v_normal);
	vec3 view = normalize(v_position - cameraPosition);
	
	vec3 color = materialColor;
	float fresnel = abs(dot(normal, -view));
	color *= 1 + fresnel * 20;

	out_color = vec4(color, 1);
}
