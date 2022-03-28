#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;


uniform mat4 lightSpaceMat;
uniform mat4 model;
uniform vec3 pos;
uniform float landSize;
uniform float max_height;
uniform sampler2D texture_height;

vec2 getUV(vec3 pos){
    return (pos.xz+landSize/2.0)/landSize;
}

float getHeight(vec3 pos){
    return max_height * texture(texture_height,getUV(pos)).r;
}

void main()
{
    vec3 vPos = pos;
    vPos.y += getHeight(vPos);
    vPos += (model*vec4(aPos,1.0)).xyz;

    gl_Position = lightSpaceMat * vec4(vPos,1.0);
}