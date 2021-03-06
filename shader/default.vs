#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
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
    vs_out.FragPos = (view * vec4(vPos,1.0)).xyz;
    vs_out.TexCoords = aTexCoords;
    vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
    vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
    vs_out.TBN = mat3(T,B,N);
    gl_Position = projection * vec4(vs_out.FragPos,1.0);
}