#version 460

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec3 v_localpos;
layout (location = 3) in flat uint v_color;
layout (location = 4) in flat uint v_axis;

layout (location = 0) out vec4 out_position;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec3 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_palette;


vec3 getColorFromID(uint id, ivec3 position, uint axis)
{
	//return id == 1 ? vec3(0.3, 0.3, 0.3) : vec3(0, 1, 0);
	float x = id % 4;
	x *= 16;
	x += (axis == 0 ? position.z : position.x) % 16;
	x += 0.5;

	float y = id / 4;
	y *= 16;
	y += (axis == 1 ? position.z : position.y) % 16;
	y += 0.5;

	vec2 uv = vec2(x, y);
	uv /= 64;

	return texture(s_palette, uv).rgb;
	//uint rgba = hash(id);
	//uint r = rgba & 0xFF;
	//uint g = (rgba & 0xFF00) >> 8;
	//uint b = (rgba & 0xFF0000) >> 16;
	//return vec3(r / 255.0, g / 255.0, b / 255.0);
}

void main()
{
	if (v_localpos.x > 1 || v_localpos.y > 1 || v_localpos.z > 1)
		discard;

	ivec3 blockPosition = ivec3(floor(v_position));

	out_position = vec4(v_position, 1);
	out_normal = v_normal;
	out_color = getColorFromID(v_color, blockPosition, v_axis);
}
