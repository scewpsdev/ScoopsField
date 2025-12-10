#version 460

layout (location = 0) in vec3 v_texcoord;
layout (location = 1) in vec4 v_color;

layout (location = 0) out vec4 FragColor;

layout(set = 2, binding = 0) uniform sampler2D s_texture0;
layout(set = 2, binding = 1) uniform sampler2D s_texture1;
layout(set = 2, binding = 2) uniform sampler2D s_texture2;
layout(set = 2, binding = 3) uniform sampler2D s_texture3;
layout(set = 2, binding = 4) uniform sampler2D s_texture4;
layout(set = 2, binding = 5) uniform sampler2D s_texture5;
layout(set = 2, binding = 6) uniform sampler2D s_texture6;
layout(set = 2, binding = 7) uniform sampler2D s_texture7;
layout(set = 2, binding = 8) uniform sampler2D s_texture8;
layout(set = 2, binding = 9) uniform sampler2D s_texture9;
layout(set = 2, binding = 10) uniform sampler2D s_texture10;
layout(set = 2, binding = 11) uniform sampler2D s_texture11;
layout(set = 2, binding = 12) uniform sampler2D s_texture12;
layout(set = 2, binding = 13) uniform sampler2D s_texture13;
layout(set = 2, binding = 14) uniform sampler2D s_texture14;
layout(set = 2, binding = 15) uniform sampler2D s_texture15;


vec4 SampleTexture(int id, vec2 texcoord)
{
	switch(id)
	{
		case 0: return texture(s_texture0, texcoord);
		case 1: return texture(s_texture1, texcoord);
		case 2: return texture(s_texture2, texcoord);
		case 3: return texture(s_texture3, texcoord);
		case 4: return texture(s_texture4, texcoord);
		case 5: return texture(s_texture5, texcoord);
		case 6: return texture(s_texture6, texcoord);
		case 7: return texture(s_texture7, texcoord);
		case 8: return texture(s_texture8, texcoord);
		case 9: return texture(s_texture9, texcoord);
		case 10: return texture(s_texture10, texcoord);
		case 11: return texture(s_texture11, texcoord);
		case 12: return texture(s_texture12, texcoord);
		case 13: return texture(s_texture13, texcoord);
		case 14: return texture(s_texture14, texcoord);
		case 15: return texture(s_texture15, texcoord);
		default: return vec4(1, 0, 1, 1);
	}
}

void main()
{
	int textureID = int(v_texcoord.z);
	vec4 textureColor = textureID >= 0 ? SampleTexture(textureID, v_texcoord.xy) : vec4(1);
	FragColor = v_color * textureColor;
}