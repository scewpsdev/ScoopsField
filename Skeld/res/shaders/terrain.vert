#version 460

//layout (location = 0) in float a_height;
//layout (location = 1) in vec2 a_normal;

layout (location = 0) out vec3 v_normal;
layout (location = 1) out vec2 v_texcoord;
layout (location = 2) out vec4 v_materials;

layout (set = 0, binding = 0) uniform sampler2D s_heightmap;
layout (set = 0, binding = 1) uniform sampler2D s_normalmap;
layout (set = 0, binding = 2) uniform usampler2D s_materialmap;

layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionViewModel;
	mat4 u_view;
	mat4 u_projection;
	mat4 u_model;
	vec4 params;

#define u_time params.x
#define viewSpaceBuffer params.y
};

#define TILE_SIZE 2.0
#define VERTICES 17


void main()
{
	float x = (gl_VertexIndex % VERTICES) * TILE_SIZE;
	float z = (gl_VertexIndex / VERTICES) * TILE_SIZE;

	vec2 heightmapCoord = ((vec2(x, z) / TILE_SIZE) + 0.5) / VERTICES;
	float height = textureLod(s_heightmap, heightmapCoord, 0).x;
	vec3 position = vec3(x, height, z);

	vec3 normal = textureLod(s_normalmap, heightmapCoord, 0).xyz;
	normal = normalize(vec3(normal.x, 1, normal.y));
	vec4 viewSpaceNormal = vec4(normal, 0);
	if (viewSpaceBuffer > 0.5) viewSpaceNormal = u_view * viewSpaceNormal;
	v_normal = viewSpaceNormal.xyz;

	uint material = textureLod(s_materialmap, heightmapCoord, 0).r;
	vec4 materialWeights = vec4(
		float(material & 1),
		float(material & 2),
		float(material & 4),
		float(material & 8)
	);
	v_materials = materialWeights;

	vec3 worldPosition = u_model[3].xyz + position;
	vec2 texcoord = worldPosition.xz / 5;
	v_texcoord = texcoord;

	gl_Position = u_projectionViewModel * vec4(position, 1);
}
