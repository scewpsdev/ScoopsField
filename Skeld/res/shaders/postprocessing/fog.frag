#version 460

layout (location = 0) in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

layout (set = 2, binding = 0) uniform sampler2D s_depth;
layout (set = 2, binding = 1) uniform samplerCube s_skyCube;

layout(set = 3, binding = 0) uniform UniformBlock {
	mat4 projectionViewInv;
	vec4 params;
    vec4 params2;

#define cameraPosition params.xyz
#define sunDirection params2.xyz
};


vec3 reconstructPosition(vec2 uv, float depth)
{
	vec4 ndc = vec4(uv.x * 2 - 1, uv.y * -2 + 1, depth, 1);
	vec4 worldPosition = projectionViewInv * ndc;
	return worldPosition.xyz / worldPosition.w;
}

void main()
{
    float depth = texture(s_depth, v_texcoord).r;
    float alpha = 0;

    if (depth > 0)
    {
        float fogDensity = 0.0001;
        float dist = 1.0 / depth;
        float fog = exp(-dist * fogDensity);
        alpha = 1 - fog;
    }

    vec3 position = reconstructPosition(v_texcoord, depth); // world space position
	vec3 view = normalize(cameraPosition - position); // world space view

    float fogBrightness = mix(1, 10, smoothstep(-0.1, 0.1, -sunDirection.y));
    vec3 fogColor = textureLod(s_skyCube, vec3(0, 0, 1), 15).rgb; // * fogBrightness;
    
    out_color = vec4(fogColor, alpha);
}