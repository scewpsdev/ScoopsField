#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_normal;
layout(set = 2, binding = 1) uniform sampler2D s_color;
layout(set = 2, binding = 2) uniform sampler2D s_depth;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 lightData0;
	vec4 lightData1;

#define lightDirection lightData0.xyz
#define lightColor lightData1.rgb
};


void main()
{
	float depth = texture(s_depth, v_texcoord).r;
	if (depth == 1)
		discard;

	vec3 normal = texture(s_normal, v_texcoord).rgb;
	vec3 color = texture(s_color, v_texcoord).rgb;

	float d = dot(normal, -lightDirection) * 0.5 + 0.5;
	vec3 diffuse = d * color * lightColor;

	out_color = vec4(diffuse, 1);
}
