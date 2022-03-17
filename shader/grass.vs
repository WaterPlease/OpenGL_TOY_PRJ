#version 430 core

layout (location = 0) in vec3 aPos;

uniform int numAxisPatches;
uniform float patchSize;

void main(void)
{
    float xpos = 2*patchSize*(mod(gl_InstanceID,numAxisPatches)-(numAxisPatches/2));
    float zpos = 2*patchSize*((gl_InstanceID/numAxisPatches)-(numAxisPatches/2));

    gl_Position = vec4(vec3(xpos,0.0,zpos)+aPos, 1.0);
}