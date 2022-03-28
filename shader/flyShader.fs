#version 430 core

layout (location = 0) out vec4 gPositionMetal;
layout (location = 1) out vec4 gNormalRough;
layout (location = 2) out vec4 gAlbedoAO;

out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec3 Color;
} fs_in;

uniform mat4 view;

void main()
{
    gPositionMetal.xyz = fs_in.FragPos;
    gPositionMetal.w   = 0.0;

    gNormalRough.xyz = vec3(0.0);
    gNormalRough.w   = 1.0;

    gAlbedoAO.rgb = fs_in.Color;
    gAlbedoAO.a = 1.0;
}