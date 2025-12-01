#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_position;
layout(set = 2, binding = 1) uniform sampler2D s_normal;
layout(set = 2, binding = 2) uniform sampler2D s_color;
layout(set = 2, binding = 3) uniform sampler2D s_depth;


float depthToDistance(float depth, float near, float far)
{
	depth = depth * 2 - 1;
	return 2.0 * near * far / (far + near - depth * (far - near));
}

void main()
{
	vec4 positionW = texture(s_position, v_texcoord);
	if (positionW.w < 0.5)
		discard;

	vec3 position = positionW.xyz;
	vec3 normal = texture(s_normal, v_texcoord).rgb;
	vec3 color = texture(s_color, v_texcoord).rgb;
	float depth = texture(s_depth, v_texcoord).r;

	vec3 toLight = normalize(vec3(1, 2, 0.5));
	float d = dot(normal, toLight) * 0.5 + 0.5;
	vec3 diffuse = d * color;

	float fogDensity = 0.00015;
	float dist = depthToDistance(depth, 1, 8000);
	float visibility = exp(-dist * fogDensity);

	vec3 fogColor = vec3(0.4, 0.4, 1.0);
	vec3 final = mix(fogColor, diffuse, visibility);
	
	out_color = vec4(final, 1);
}
