#version 430 core
layout (location = 0) in vec3 aPos;

#define EPS 0.001

layout(std430, binding = 3) buffer flyInfo{
	float pos[32*32*5];
}flyinfo;

out VS_OUT{
    vec3 FragPos;
    vec3 Color;
} vs_out;

uniform mat4 view;
uniform mat4 projection;
uniform float landSize;
uniform float max_height;
uniform float waterLevel;
uniform sampler2D texture_height;


void main()
{
    vec3 vPos = vec3(flyinfo.pos[5*gl_InstanceID+0],flyinfo.pos[5*gl_InstanceID+1],flyinfo.pos[5*gl_InstanceID+2]);
    //vPos.y += max(getHeight(vPos),waterLevel);
    vPos += aPos.xyz;
    vs_out.FragPos = (view * vec4(vPos,1.0)).xyz;
    vec3 color = vec3(0.2);
    color[uint(flyinfo.pos[5*gl_InstanceID+4])] = 1.2;
    vs_out.Color = (color)*flyinfo.pos[5*gl_InstanceID+3];
    gl_Position = projection * vec4(vs_out.FragPos,1.0);
}