#version 430 core

layout (location = 0) out vec4 gPositionMetal;
layout (location = 1) out vec4 gNormalRough;
layout (location = 2) out vec4 gAlbedoAO;

out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform mat4 view;
uniform sampler2D texture_albedo1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_metal1;
uniform sampler2D texture_rough1;

void main()
{    
    vec3 normal = texture(texture_normal1,fs_in.TexCoords).rgb*2.0-1.0;
    normal = normalize(fs_in.TBN * normal);

    gPositionMetal.xyz = fs_in.FragPos;
    gPositionMetal.w   = texture(texture_metal1, fs_in.TexCoords).r;

    gNormalRough.xyz = (view * vec4(normal,0.0)).xyz;
    gNormalRough.w   = texture(texture_rough1, fs_in.TexCoords).r;

    gAlbedoAO.rgb = texture(texture_albedo1, fs_in.TexCoords).rgb;
    gAlbedoAO.a = 1.0;
}