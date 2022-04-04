#version 430 core

layout (location = 0) out vec4 gPositionMetal;
layout (location = 1) out vec4 gNormalRough;
layout (location = 2) out vec4 gAlbedoAO;

#define EPS 0.001

uniform sampler2D texture_albedo_grass;
uniform sampler2D texture_albedo_rock;

uniform sampler2D texture_normal_grass;
uniform sampler2D texture_normal_rock;

uniform sampler2D texture_rough_grass;
uniform sampler2D texture_rough_rock;

uniform sampler2D texture_ao_grass;
uniform sampler2D texture_ao_rock;

uniform sampler2D texture_shadow;

uniform float uvFactorRock;
uniform float uvFactorGrass;
uniform float grassCrit;

out vec4 FragColor;

in FGINFO
{
    vec2 uv;
    mat3 TBN;
    vec3 pos;
}fginfo;
uniform mat4 view;

void main()
{   
    float blend = 0.1;
    float rockStart = grassCrit;
        
    vec3 upVec = vec3(0.0,1.0,0.0);
    float planeWeight = clamp(dot(upVec,fginfo.TBN*vec3(0.0,0.0,1.0)),0.0,1.0);

    float rockWeight = smoothstep(0.0,1.0,(rockStart-planeWeight)/blend);

    vec3 normal_grass = texture(texture_normal_grass,uvFactorGrass*fginfo.uv).rgb;
    normal_grass = normal_grass * 2.0 - 1.0;
    normal_grass = normalize(fginfo.TBN * normal_grass);
    vec3 normal_rock = texture(texture_normal_rock,uvFactorRock*fginfo.uv).rgb;
    normal_rock = normal_rock * 2.0 - 1.0;
    normal_rock = normalize(fginfo.TBN * normal_rock);
    vec3 normal = mix(normal_grass, normal_rock,rockWeight);

    vec3 color_grass = texture(texture_albedo_grass,uvFactorGrass*fginfo.uv).rgb;
    vec3 color_rock = texture(texture_albedo_rock,uvFactorRock*fginfo.uv).rgb;
    vec3 albedo = mix(color_grass, color_rock,rockWeight);

    float rough_grass = texture(texture_rough_grass,uvFactorGrass*fginfo.uv).r;
    float rough_rock  = texture(texture_rough_rock,uvFactorRock*fginfo.uv).r;
    float rough = mix(rough_grass, rough_rock,rockWeight);
   
    float ao_grass = texture(texture_ao_grass,uvFactorGrass*fginfo.uv).r;
    float ao_rock  = texture(texture_ao_rock,uvFactorRock*fginfo.uv).r;
    float ao = mix(ao_grass, ao_rock,rockWeight);

    gPositionMetal.xyz = (view*vec4(fginfo.pos,1.0)).xyz;
    gPositionMetal.w = 0.0;

    gNormalRough.xyz = normalize((view*vec4(normalize(normal),0.0)).xyz);
    gNormalRough.w = rough;
    gAlbedoAO.rgb = albedo.rgb;
    gAlbedoAO.a = ao;

}