#version 460

//layout (location = 0) in vec3 a_position;
//layout (location = 1) in vec2 a_texcoord;

layout (location = 0) out vec2 v_texcoord;
layout (location = 1) out vec4 v_color;
layout (location = 2) out vec4 v_bgcolor;


struct GlyphData
{
	vec2 position;
	vec2 padding;
	vec4 rect;
	vec4 color;
    vec4 bgcolor;
};

layout(std140, set = 0, binding = 0) readonly buffer GlyphBuffer {
    GlyphData u_glyphs[];
};

layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 u_projection;
};

const int indices[6] = {0, 1, 2, 2, 3, 0};
const vec2 positions[4] = {
    {0, 16},
    {8, 16},
    {8, 0},
    {0, 0}
};


void main()
{
    int vertexIdx = indices[gl_VertexIndex % 6];
    int glyphID = gl_VertexIndex / 6;
    GlyphData glyph = u_glyphs[glyphID];

    vec2 position = glyph.position + positions[vertexIdx];

    vec2 texcoords[4] = {
        vec2(glyph.rect.x, glyph.rect.y + glyph.rect.w),
        vec2(glyph.rect.x + glyph.rect.z, glyph.rect.y + glyph.rect.w),
        vec2(glyph.rect.x + glyph.rect.z, glyph.rect.y),
        vec2(glyph.rect.x, glyph.rect.y)
    };

    gl_Position = u_projection * vec4(position, 0.0f, 1.0f);

    v_texcoord = texcoords[vertexIdx];
    v_color = glyph.color;
    v_bgcolor = glyph.bgcolor;
}