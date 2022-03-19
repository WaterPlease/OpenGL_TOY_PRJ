#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 fragCoord;

uniform mat4 view;
uniform mat4 projection;
uniform float waterLevel;

void main()
{
    TexCoords = aTexCoords;    
    fragCoord = aPos;
    gl_Position = projection * view * (vec4(aPos, 1.0)+vec4(0.0,waterLevel,0.0,0.0));
}