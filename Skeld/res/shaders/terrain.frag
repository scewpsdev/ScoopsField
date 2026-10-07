#version 460

#include "common.glsl"

layout (location = 0) in vec3 v_normal;
layout (location = 1) in vec2 v_texcoord;
layout (location = 2) in vec4 v_materials;

layout (location = 0) out vec4 out_normal;
layout (location = 1) out vec3 out_color;
layout (location = 2) out vec4 out_material;
layout (location = 3) out vec4 out_emissive;


layout (set = 2, binding = 0) uniform sampler2D s_texture0;
layout (set = 2, binding = 1) uniform sampler2D s_texture1;

/*
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
*/


void main()
{
	vec3 color0 = vec3(0.22525, 0.325, 0.13);
	vec3 color1 = vec3(0.13, 0.1, 0.07);
	vec3 color2 = vec3(1, 0, 1);
	vec3 color3 = vec3(1, 0, 1);

	vec4 textureColor0 = texture(s_texture0, v_texcoord);
	vec4 textureColor1 = texture(s_texture1, v_texcoord);

	vec3 color = v_materials.x * color0 + v_materials.y * color1 + v_materials.z * color2 + v_materials.w * color3;
	vec4 textureColor = v_materials.x * textureColor0 + v_materials.y * textureColor1;
	// maybe alpha blend overlays on top?
	// textureColor = mix(textureColor, textureColor4, textureColor4.a);

	float roughness = 1;
	float metallic = 0;
	vec3 emissiveColor = vec3(0);
	float emissiveStrength = 0;

	out_normal = vec4(normalize(v_normal) * 0.5 + 0.5, emissiveStrength);
	out_color = textureColor.rgb * color.rgb;
	out_material = vec4(roughness, metallic, 0, 0);
	out_emissive = vec4(linearToSRGB(emissiveColor), 0);
}
