#version 460

//layout (location = 0) in vec3 a_position;
//layout (location = 1) in vec2 a_texcoord;

layout (location = 0) out vec2 v_texcoord;
layout (location = 1) out vec4 v_color;


struct SpriteData
{
	vec3 position;
	float rotation;
	vec2 scale;
	vec2 padding;
	vec4 rect;
	vec4 color;
};

layout(std140, set = 0, binding = 0) readonly buffer SpriteBuffer {
    SpriteData u_sprites[];
};

layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projectionView;
};

const int indices[6] = {0, 1, 2, 2, 3, 0};
const vec2 positions[4] = {
    {-0.5f, -0.5f},
    {0.5f, -0.5f},
    {0.5f, 0.5f},
    {-0.5f, 0.5f}
};


void main()
{
    int vertexIdx = indices[gl_VertexIndex % 6];
    int spriteID = gl_VertexIndex / 6;
    SpriteData sprite = u_sprites[spriteID];

    float s = sin(sprite.rotation);
    float c = cos(sprite.rotation);

    vec3 position = vec3(positions[vertexIdx], 0);
    position.xy *= sprite.scale;

    position.xy = vec2(position.x * c - position.y * s, position.y * c + position.x * s); // rotation

    position += sprite.position;

    vec2 texcoords[4] = {
        vec2(sprite.rect.x, sprite.rect.y + sprite.rect.w),
        vec2(sprite.rect.x + sprite.rect.z, sprite.rect.y + sprite.rect.w),
        vec2(sprite.rect.x + sprite.rect.z, sprite.rect.y),
        vec2(sprite.rect.x, sprite.rect.y)
    };

    gl_Position = u_projectionView * vec4(position, 1.0f);

    v_texcoord = texcoords[vertexIdx];
    v_color = sprite.color;
}