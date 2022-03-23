#version 430 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 TexCoords;

uniform samplerCube skybox;

void main(){

    gPosition = vec3(0.0);
    gNormal = vec3(0.0);
    gAlbedoSpec.rgb = texture(skybox,TexCoords).rgb;
    gAlbedoSpec.a   = 1.0;
}