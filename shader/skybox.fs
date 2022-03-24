#version 430 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 TexCoords;
in vec3 fragPos;

uniform samplerCube skybox;

void main(){
    gPosition = vec4(fragPos,0.0);
    gNormal = vec4(0.0);
    gAlbedoSpec.rgb = texture(skybox,TexCoords).rgb;
    gAlbedoSpec.a   = 1.0;
}