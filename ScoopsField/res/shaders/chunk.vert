#version 460

layout (location = 0) in uint in_data;

layout (location = 0) out vec3 v_position;
layout (location = 1) out vec3 v_normal;
layout (location = 2) out vec3 v_localpos;
layout (location = 3) out flat uint v_color;
layout (location = 4) out flat uint v_axis;

layout(set = 0, binding = 0) readonly buffer ChunkData {
	ivec4 chunkData[];
};

layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 pv;
	//int chunkID;
};


const vec3 _vertices[6 * 3] = vec3[6 * 3](
	// left
	vec3(0, 2, 0),
	vec3(0, 0, 0),
	vec3(0, 0, 2),

	// right
	vec3(1, 0, 2),
	vec3(1, 0, 0),
	vec3(1, 2, 0),

	// down
	vec3(0, 0, 2),
	vec3(0, 0, 0),
	vec3(2, 0, 0),

	// up
	vec3(2, 1, 0),
	vec3(0, 1, 0),
	vec3(0, 1, 2),

	// forward
	vec3(2, 0, 0),
	vec3(0, 0, 0),
	vec3(0, 2, 0),

	// back
	vec3(0, 2, 1),
	vec3(0, 0, 1),
	vec3(2, 0, 1)
);

const vec3 vertices[3] = vec3[3](
	vec3(0, 0, 0),
	vec3(0, 2, 0),
	vec3(2, 0, 0)
);

const vec3 normals[6] = vec3[6](
	vec3(-1, 0, 0),
	vec3(1, 0, 0),
	vec3(0, -1, 0),
	vec3(0, 1, 0),
	vec3(0, 0, -1),
	vec3(0, 0, 1)
);


uint hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

void main()
{
	uint x = in_data & 0x1F;
	uint y = (in_data >> 5) & 0x1F;
	uint z = (in_data >> 10) & 0x1F;
	uint sx = ((in_data >> 15) & 0x1F) + 1;
	uint sy = ((in_data >> 20) & 0x1F) + 1;
	uint face = (in_data >> 25) & 0x07;
	uint color = (in_data >> 28) & 0x0F;

	ivec3 blockPosition = ivec3(x, y, z);

	//vec3 vertexPosition = vertices[face * 3 + gl_VertexIndex % 3]; //vec3(1 - gl_VertexIndex / 2, 0, gl_VertexIndex % 2);
	vec3 localPos = vertices[gl_VertexIndex % 3];
	vec3 vertexPosition = localPos;

	uint axis = face / 2;
	uint sgn = face % 2;

	vertexPosition.xy = (axis == 0 || axis == 1) != (sgn == 1) ? vertexPosition.yx : vertexPosition.xy;
	vertexPosition.xy *= vec2(sx, sy);
	vertexPosition.z += sgn;
	vertexPosition = axis == 0 ? vertexPosition.zyx : axis == 1 ? vertexPosition.xzy : vertexPosition;

	//vec3 scaledVertexPosition = vertexPosition; // * vec3(sx, sy, sx);

	//if (face == 0)
	//	vertexPosition = vertexPosition.yzx;
	//else if (face == 1)
	//	vertexPosition = vertexPosition.yxz + vec3(1, 0, 0);
	//else if (face == 3)
	//	vertexPosition = vertexPosition.zyx + vec3(0, 1, 0);
	//else if (face == 4)
	//	vertexPosition = vertexPosition.zxy;
	//else if (face == 5)
	//	vertexPosition = vertexPosition.xzy + vec3(0, 0, 1);

	ivec4 chunkPositionScale = chunkData[gl_DrawID];
	ivec3 chunkPosition = chunkPositionScale.xyz;
	int chunkSize = chunkPositionScale.w;

	vec3 worldPosition = chunkPosition + (blockPosition + vertexPosition) * chunkSize;

	gl_Position = pv * vec4(worldPosition, 1);

	v_position = worldPosition;
	v_normal = normals[face];
	v_localpos = localPos;
	v_color = color;
	v_axis = axis;
}
