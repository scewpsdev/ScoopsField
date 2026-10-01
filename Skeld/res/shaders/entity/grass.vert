#version 460

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec4 i_blade;


layout(set = 0, binding = 0) uniform sampler2D s_heightmap;

layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
	mat4 u_view;
	mat4 u_projection;
	mat4 u_model;
	vec4 params;

	//vec4 grassData;

#define u_time params.x
#define u_viewSpaceBuffer params.y

//#define u_lod grassData.x
};


#define TILE_SIZE 2.0
#define VERTICES 33


void main()
{
	vec2 terrainPosition = i_blade.xy;
	float rotation = i_blade.z;
	float scale = i_blade.w;

	vec2 heightmapCoord = ((terrainPosition / TILE_SIZE) + 0.5) / VERTICES;
	float height = textureLod(s_heightmap, heightmapCoord, 0).x;

	float s = sin(rotation);
	float c = cos(rotation);
	vec3 vertexPosition = vec3(a_position.x * c, a_position.y, -a_position.x * s);
	//vertexPosition.xz *= pow(2, u_lod);
	//vertexPosition *= scale;

	vec3 position = vec3(terrainPosition.x, height, terrainPosition.y) + vertexPosition;

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

	gl_Position = u_projectionViewModel * vec4(position, 1);
}
