#version 430 core

layout (location = 0) out vec4 gPositionMetal;
layout (location = 1) out vec4 gNormalRough;
layout (location = 2) out vec4 gAlbedoAO;

in vec3 TexCoords;
in vec3 fragPos;

uniform samplerCube skybox;

void main(){
    gPositionMetal = vec4(fragPos,0.0);
    gNormalRough = vec4(0.0);
    gAlbedoAO.rgb = texture(skybox,TexCoords).rgb;
    gAlbedoAO.a   = -1.0;
}