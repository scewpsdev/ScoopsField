#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout (set = 2, binding = 0) uniform sampler2D s_texture;


void main()
{
    float depth = texture(s_texture, v_texcoord).r;
    float alpha = 0;

    if (depth > 0)
    {
        float dist = 1.0 / depth;
        float fog = exp(-dist * 0.00001);
        alpha = 1 - fog;
    }
    
    out_color = vec4(1, 1, 1, alpha);
}