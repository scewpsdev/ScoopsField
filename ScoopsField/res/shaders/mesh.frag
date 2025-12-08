#version 460

#include "common.shader"

layout (location = 0) in vec3 v_normal;
layout (location = 1) in vec2 v_texcoord;

layout (location = 0) out vec4 out_normal;
layout (location = 1) out vec3 out_color;


layout(set = 2, binding = 0) uniform sampler2D s_diffuse;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 materialData0;
	vec4 materialData1;

#define hasDiffuse materialData0.x
#define materialColor materialData1.rgb
};


void main()
{
	vec4 textureColor = texture(s_diffuse, v_texcoord);
	textureColor.rgb = SRGBToLinear(mix(vec3(1), textureColor.rgb, hasDiffuse));

	out_normal = vec4(normalize(v_normal), 1);
	out_color = textureColor.rgb * materialColor;
}
