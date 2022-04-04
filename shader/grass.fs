#version 430 core

layout (location = 0) out vec4 gPositionMetal;
layout (location = 1) out vec4 gNormalRough;
layout (location = 2) out vec4 gAlbedoAO;

#define GRASS_COLOR vec3(0.0,0.7,0.0)

#define EPS 0.001
#define M_PI 3.1415926535897932384626433832795
#define SHADOW_SAMPLE 64
#define SHADOW_SAMPLE_HALF 32
#define SHADOW_SAMPLE_INV (1.0/SHADOW_SAMPLE)
#define SHADOW_SAMPLE_SQRT 8

in vec3 worldPos;
in vec2 texCoords;
in vec3 fNormal;
in vec3 viewDir;

uniform mat4 view;
uniform sampler2D texture_grass;

out vec4 FragColor;

void main()
{   
    vec4 color = texture(texture_grass,texCoords);
    if(color.a < 0.1     ||
       texCoords.y < 0.3 || 
       texCoords.x<0.1   ||
       texCoords.x>0.95  ||
       abs(dot(viewDir,fNormal))<0.1)
        discard;

    gPositionMetal.xyz = (view*vec4(worldPos,1.0)).xyz;
    gPositionMetal.w = 0.0;

    gNormalRough.xyz = normalize((view*vec4(0.0,1.0,0.0,0.0)).xyz);
    gNormalRough.w = 1.0;

    gAlbedoAO.rgb = color.rgb*1.2;
    gAlbedoAO.a   = 1.0;
}