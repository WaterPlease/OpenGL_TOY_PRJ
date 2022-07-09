#version 430 core

#define M_PI 3.1415926535897932384626433832795
#define EPS 0.001

#define DENSITY 128

layout(local_size_x = 1, local_size_y=1, local_size_z=1) in;
layout(std430, binding = 3) volatile buffer flyInfo {
	float pos [DENSITY*DENSITY*5];
}flyinfo;

uniform float t;
uniform float landSize;
uniform float max_height;
uniform float waterLevel;
uniform sampler2D texture_height;
float speed = 0.1;

float F = 8;

vec2 getUV(vec3 pos){
    return (pos.xz+landSize/2.0)/landSize;
}

float getHeight(vec3 pos){
    return max_height * texture(texture_height,getUV(pos)).r;
}

void main(){
    int idx = int(gl_GlobalInvocationID.x* DENSITY + gl_GlobalInvocationID.y);

    float rPhaseDiff = fract(sin(dot(gl_GlobalInvocationID.xy, vec2(12.9898, 78.233))) * 43758.5453);
    float freqRandom = rPhaseDiff;
    rPhaseDiff = fract(sin(dot(vec2(rPhaseDiff), vec2(12.9898, 78.233))) * 43758.5453);

    float phase = 2.0*M_PI*t*speed + rPhaseDiff;
    vec2 xzPos = vec2(gl_GlobalInvocationID.x,gl_GlobalInvocationID.y)/ DENSITY - vec2(0.5);
    xzPos = xzPos*landSize;

    flyinfo.pos[5*idx+0] = mix(sin(3.0*phase*freqRandom),cos(3.0*phase*freqRandom),rPhaseDiff)+xzPos.x;
    flyinfo.pos[5*idx+1] = abs(mix(cos(phase*freqRandom),sin(phase*freqRandom),rPhaseDiff));
    flyinfo.pos[5*idx+2] = mix(cos(5.0*phase*freqRandom),cos(5.0*phase*freqRandom),rPhaseDiff)+xzPos.y;

    rPhaseDiff = fract(sin(dot(vec2(rPhaseDiff), vec2(12.9898, 78.233))) * 43758.5453);
    flyinfo.pos[5*idx+3] = sin(2.0*M_PI*t*freqRandom+rPhaseDiff)*0.25+0.75;
    flyinfo.pos[5*idx+4] = mod(float(idx),3.0);

    
    flyinfo.pos[5*idx+1] += max(getHeight(vec3(flyinfo.pos[5*idx+0],flyinfo.pos[5*idx+1],flyinfo.pos[5*idx+2])),waterLevel);
};