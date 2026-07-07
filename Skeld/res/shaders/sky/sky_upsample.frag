#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_sky;

layout(set = 3, binding = 0) uniform UniformBlock {
	mat4 projectionInv;
	mat4 viewInv;
};


#include "../common.glsl"


vec3 reconstructView(vec2 uv, mat4 projectionInv, mat4 viewInv)
{
	vec2 ndc = vec2(uv.x * 2 - 1, uv.y * -2 + 1);

	vec3 dir;
	dir.x = ndc.x * projectionInv[0][0];
	dir.y = ndc.y * projectionInv[1][1];
	dir.z = -1;

	dir = mat3(viewInv) * dir;
	dir = normalize(dir);

	return dir;
}

vec3 sampleSkyViewLUT(vec3 dir)
{
	float longitude = mod(atan(dir.x, dir.z) + 2 * pi, 2 * pi);
	float latitude = asin(dir.y);

	float u = longitude / pi * 0.5;
	float v = 0.5 + 0.5 * -sign(latitude) * sqrt(abs(latitude) / pi * 2);

	vec3 color = texture(s_sky, vec2(u, v)).rgb;
	color = SRGBToLinear(color);

	return color;
}

void main()
{
	vec3 dir = reconstructView(v_texcoord, projectionInv, viewInv);
	vec3 color = sampleSkyViewLUT(dir);

	out_color = vec4(color, 0);
}
