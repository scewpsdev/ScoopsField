#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 i_position;
layout (location = 2) in vec4 i_color;

layout (location = 0) out vec3 v_lightPosition;
layout (location = 1) out vec3 v_lightColor;
layout (location = 2) out vec4 v_maskPlanes[5];

layout(std140, set = 1, binding = 0) uniform UniformBlock {
    mat4 projectionView;
    mat4 view;
    vec4 portalMask[4];
};


vec4 createPlane(vec3 normal, vec3 point)
{
    normal = normalize(normal);
    return vec4(normal, -dot(normal, point));
}

void main()
{
    vec3 worldPosition = a_position * i_position.w + i_position.xyz;

    gl_Position = projectionView * vec4(worldPosition, 1.0);

    vec3 L = (view * vec4(i_position.xyz, 1)).xyz;

    v_lightPosition = L;
    v_lightColor = i_color.rgb;

    if (portalMask[0].w > 0.5)
    {
        vec3 V0 = portalMask[0].xyz;
        vec3 V1 = portalMask[1].xyz;
        vec3 V2 = portalMask[2].xyz;
        vec3 V3 = portalMask[3].xyz;

        vec3 maskNormal = cross(V1 - V0, V2 - V0);
        v_maskPlanes[0] = createPlane(maskNormal, V0);

        v_maskPlanes[1] = createPlane(cross(V0 - L, V1 - V0), V0);
        v_maskPlanes[2] = createPlane(cross(V1 - L, V2 - V1), V1);
        v_maskPlanes[3] = createPlane(cross(V2 - L, V3 - V2), V2);
        v_maskPlanes[4] = createPlane(cross(V3 - L, V0 - V3), V3);
    }
    else
    {
        v_maskPlanes[0] = vec4(0, 0, 0, 1);
        v_maskPlanes[1] = vec4(0, 0, 0, 1);
        v_maskPlanes[2] = vec4(0, 0, 0, 1);
        v_maskPlanes[3] = vec4(0, 0, 0, 1);
        v_maskPlanes[4] = vec4(0, 0, 0, 1);
    }
}