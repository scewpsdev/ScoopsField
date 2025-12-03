#version 460

layout (location = 0) in vec2 v_texcoord;
layout (location = 1) in vec3 v_lightPosition;
layout (location = 2) in vec3 v_lightColor;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_normal;
layout(set = 2, binding = 1) uniform sampler2D s_color;
layout(set = 2, binding = 2) uniform sampler2D s_depth;


vec3 reconstructPosition(float dist)
{

}

void main()
{
	float depth = texture(s_depth, v_texcoord).r;
	if (depth == 1)
		discard;

	float dist = depthToDistance(depth, cameraNear, cameraFar);
	vec3 position = reconstructPosition(dist);

	vec3 normal = texture(s_normal, v_texcoord).rgb;
	vec3 color = texture(s_color, v_texcoord).rgb;

	vec3 toLight = normalize(v_lightPosition - position);
	float d = max(dot(normal, toLight), 0);
	vec3 diffuse = d * color * v_lightColor;
	
	out_color = vec4(diffuse, 1);
}
