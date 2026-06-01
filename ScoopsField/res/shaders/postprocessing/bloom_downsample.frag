#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

#include "../common.glsl"


layout(set = 2, binding = 0) uniform sampler2D s_input;


void main()
{
	vec2 texelSize = 1.0 / textureSize(s_input, 0);
	float x = texelSize.x;
	float y = texelSize.y;

	vec3 a = texture(s_input, vec2(v_texcoord.x - 2 * x, v_texcoord.y + 2 * y)).rgb;
	vec3 b = texture(s_input, vec2(v_texcoord.x, v_texcoord.y + 2 * y)).rgb;
	vec3 c = texture(s_input, vec2(v_texcoord.x + 2 * x, v_texcoord.y + 2 * y)).rgb;

	vec3 d = texture(s_input, vec2(v_texcoord.x - 2 * x, v_texcoord.y)).rgb;
	vec3 e = texture(s_input, vec2(v_texcoord.x, v_texcoord.y)).rgb;
	vec3 f = texture(s_input, vec2(v_texcoord.x + 2 * x, v_texcoord.y)).rgb;

	vec3 g = texture(s_input, vec2(v_texcoord.x - 2 * x, v_texcoord.y - 2 * y)).rgb;
	vec3 h = texture(s_input, vec2(v_texcoord.x, v_texcoord.y - 2 * y)).rgb;
	vec3 i = texture(s_input, vec2(v_texcoord.x + 2 * x, v_texcoord.y - 2 * y)).rgb;

	vec3 j = texture(s_input, vec2(v_texcoord.x - x, v_texcoord.y + y)).rgb;
	vec3 k = texture(s_input, vec2(v_texcoord.x + x, v_texcoord.y + y)).rgb;
	vec3 l = texture(s_input, vec2(v_texcoord.x - x, v_texcoord.y - y)).rgb;
	vec3 m = texture(s_input, vec2(v_texcoord.x + x, v_texcoord.y - y)).rgb;

	vec3 result = e * 0.125;
	result += (a + c + g + i) * 0.03125;
	result += (b + d + f + h) * 0.0625;
	result += (j + k + l + m) * 0.125;

	result = max(result, 0.0001);

	out_color = vec4(result, 1.0);
}
