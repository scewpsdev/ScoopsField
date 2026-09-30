#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 4) in vec2 a_texcoord;
layout (location = 5) in vec4 i_blade;

layout (location = 0) out vec3 v_normal;
layout (location = 1) out vec2 v_texcoord;


layout(set = 0, binding = 0) uniform sampler2D s_heightmap;

layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
	mat4 u_view;
	mat4 u_projection;
	mat4 u_model;
	vec4 params;

#define u_time params.x
#define u_viewSpaceBuffer params.y
};


#define TILE_SIZE 2.0
#define VERTICES 129


void main()
{
	//mat4 model = u_viewSpaceBuffer > 0.5 ? inverse(u_view) * u_model : u_model;
	vec2 heightmapCoord = ((i_blade.xy / TILE_SIZE) + 0.5) / VERTICES;
	float height = texture(s_heightmap, heightmapCoord).x;
	vec4 worldPosition = vec4(a_position + vec3(i_blade.x, height, i_blade.y), 1);

	// wind

	/*
	float heightMask = max(a_position.y, 0) * 0.25;
	heightMask = heightMask * heightMask;
	vec3 windDirection = vec3(0.7, 0, 0.7);

	float trunkWave = sin(u_time * 0.7 + (a_position.x + a_position.z) * 0.02);
	worldPosition.xyz += windDirection * trunkWave * heightMask * 0.1;

	float wave1 = sin(u_time * 1.5 + (a_position.x + a_position.z) * 0.5);
	float wave2 = cos(u_time * 1.5 * 1.8 + (a_position.x - a_position.z) * 0.5 * 1.5);
	float wind = wave1 * 0.7 + wave2 * 0.3;
	float leafMask = min(a_position.x * a_position.x + a_position.z * a_position.z, 1);
	worldPosition.xyz += windDirection * wind * heightMask * leafMask * 0.05;

	float leafFlutter = sin(u_time * 10.0 + a_position.x * 5.0);
	float flutterMask = min((a_position.x * a_position.x + a_position.z * a_position.z) * 0.2, 1);
    worldPosition.xyz += leafFlutter * heightMask * flutterMask * 0.0025;
	*/

	gl_Position = u_projectionViewModel * worldPosition;

	vec4 viewSpaceNormal = u_view * vec4(a_normal, 0);

	v_normal = viewSpaceNormal.xyz;
	v_texcoord = a_texcoord;
}
