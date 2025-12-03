#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D s_normal;
layout(set = 2, binding = 1) uniform sampler2D s_color;
layout(set = 2, binding = 2) uniform sampler2D s_depth;

layout(set = 3, binding = 0) uniform UniformBlock {
	vec4 cameraParams;
	vec4 lightData0;
	vec4 lightData1;

#define cameraNear cameraParams.x
#define cameraFar cameraParams.y
#define lightDirection lightData0.xyz
#define lightColor lightData1.rgb
};


float depthToDistance(float depth, float near, float far)
{
	depth = depth * 2 - 1;
	return 2.0 * near * far / (far + near - depth * (far - near));
}

void main()
{
	float depth = texture(s_depth, v_texcoord).r;
	if (depth == 1)
		discard;

	float dist = depthToDistance(depth, cameraNear, cameraFar);

	vec3 normal = texture(s_normal, v_texcoord).rgb;
	vec3 color = texture(s_color, v_texcoord).rgb;

	float d = dot(normal, -lightDirection) * 0.5 + 0.5;
	vec3 diffuse = d * color * lightColor;

	float fogDensity = 0.00015;
	float visibility = exp(-dist * fogDensity);

	vec3 fogColor = vec3(0.4, 0.4, 1.0);
	vec3 final = mix(fogColor, diffuse, visibility);
	
	out_color = vec4(final, 1);
}
