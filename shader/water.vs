#version 430 core
layout (location = 0) in vec3 aPos;

uniform float landSize;

void main()
{
    gl_Position = (vec4(aPos*landSize, 1.0));
}