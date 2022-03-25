#version 430 core

#define EPS (1.0/2000)
#define M_PI 3.1415926535897932384626433832795

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNormal;

uniform sampler2D texture_height;

uniform float max_height;
uniform float landSize;
uniform int axisCount;
uniform float grassSize;
uniform vec3 camFront;
uniform float cosHalfDiag;
uniform float waterLevel;
uniform vec3 camPos;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMat;
uniform float grassCrit;
uniform float grassProb;
uniform float steepness;
uniform float waveLength;
uniform float time;
uniform vec3 lightDir;

out vec3 worldPos;
out vec2 texCoords;
out vec4 lightSpacePos;
out vec3 fNormal;
out vec3 viewDir;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}
vec2 getUV(vec3 pos,float minVal,float maxVal){
    float LandAxisLen = maxVal-minVal;
    float u = (pos.x-minVal)/LandAxisLen;
    float v = (pos.z-minVal)/LandAxisLen;
    return vec2(u,v);
}
float getHeight(vec2 uv){
    return max_height * (texture(texture_height,uv).x * 0.25
                        
                        +(texture(texture_height,uv-vec2(-EPS,EPS)).x
                        +texture(texture_height,uv+vec2(-EPS,EPS)).x
                        +texture(texture_height,uv-vec2(EPS,EPS)).x
                        +texture(texture_height,uv+vec2(EPS,EPS)).x) * 0.0625

                        +(texture(texture_height,uv-vec2(EPS,0.0)).x
                        +texture(texture_height,uv+vec2(EPS,0.0)).x
                        +texture(texture_height,uv-vec2(0.0,EPS)).x
                        +texture(texture_height,uv+vec2(0.0,EPS)).x) * 0.125 );
}

vec3 trochoidal(vec3 v, vec2 dir, float steep, float wavelength,float weight){
    
	vec3 p = v;

	float k = 2 * M_PI / wavelength;
    float c = sqrt(9.8 / k);
	float f = k * (dot(dir,p.xz) - c * time);
	float a = steep / k;
	
    vec2 d = normalize(dir);

    p.x += d.x * (a * cos(f))*weight;
	p.y += a * sin(f);
	p.z += d.y * (a * cos(f))*weight;

	return p;
}

vec3 multi_trochoidal(vec3 v,float weight){
    vec3  res = trochoidal(v,normalize(vec2(1.0,-1.0)),steepness,waveLength,weight);
    res    +=     trochoidal(v,vec2(1.0,0.0),steepness*0.5,waveLength*0.5,weight);
    res    +=     trochoidal(v,normalize(vec2(0.2,0.8)),steepness*0.5,waveLength*0.5,weight);
    res *= 0.333;
    return res;
}

#define LOD2 90.0
#define LOD1 120.0
void main(void)
{   
    bool underWater = false;
    vec3 vPos = aPos*grassSize;
    float rad = rand(vec2(gl_InstanceID));
    float r1 = rand(vec2(rad));
    float r2 = rand(vec2(r1));
    int x = int(mod(gl_InstanceID,axisCount));
    int z = (gl_InstanceID/axisCount);
    vec2 xzPos = vec2(x,z)/axisCount-vec2(0.5);
    xzPos = xzPos*landSize;
    
    vec3 pos = vec3(xzPos.x,0.0,xzPos.y)+vec3(r1,0.0,r2) * 0.2;

    float minVal = -0.5*landSize;
    float maxVal = 0.5*landSize;
    
    vec2 uv = getUV(pos.xyz,minVal,maxVal);

    pos.y = getHeight(uv);
    if(pos.y < (waterLevel+0.3)){
        underWater = true;
    }

    float d = distance(camPos,pos);
    float prob = grassProb;
    if(d>LOD1){
        vPos.xz *= vec2(4.0);
        prob *= 0.25;
    }else if(d>LOD2){
        vPos.xz *= vec2(2.0);
        prob *= 0.5;
    }
    vec3 normal;
    if(!underWater){
        vec2 dpx = getUV(pos.xyz+vec3(EPS,0.0,0.0),minVal,maxVal);
        vec2 dnx = getUV(pos.xyz-vec3(EPS,0.0,0.0),minVal,maxVal);
        vec2 dpz = getUV(pos.xyz+vec3(0.0,0.0,EPS),minVal,maxVal);
        vec2 dnz = getUV(pos.xyz-vec3(0.0,0.0,EPS),minVal,maxVal);
        float RL = (getHeight(dpx)-getHeight(dnx))/EPS*0.5;
        float BT = (getHeight(dpz)-getHeight(dnz))/EPS*0.5;
        normal = normalize(vec3(RL,1.0,BT));
    }

    vec3 upVec  = vec3(0,1.0,0);
    float planeWeight = abs(dot(upVec,normal));

    // culling or discard
    float testVal_3d = dot(normalize(pos.xyz-camPos),camFront);
    if(testVal_3d < cosHalfDiag ||
       planeWeight < grassCrit  ||
       underWater         ||
       rand(vec2(r2)) > prob){//  || pos.y>2.2 ){
       gl_Position = vec4(-10000.0,-10000.0,-10000.0,1.0);
    }else{
        mat4 rot;
        float cosVal = cos(rad);
        float sinVal = sin(rad);
        rot[0] = vec4(cosVal,0.0,-sinVal,0.0);
        rot[1] = vec4(0.0,1.0,0.0,0.0);
        rot[2] = vec4(sinVal,0.0,cosVal,0.0);
        rot[3] = vec4(0.0,0.0,0.0,1.0);

        vec4 globalPos = vec4(pos + (rot * vec4(vPos,1.0)).xyz ,1.0);
        globalPos.xyz = (vPos.y > EPS)? multi_trochoidal(globalPos.xyz,1.0) : globalPos.xyz;

        texCoords = aTex;
        lightSpacePos = lightSpaceMat * globalPos;
        gl_Position = projection * view * globalPos;
        fNormal = normalize((rot * vec4(aNormal,0.0)).xyz);
        //if(dot(fNormal,lightDir)<0)
        //    fNormal = -fNormal;
        viewDir = normalize(camPos-globalPos.xyz);
        worldPos = globalPos.xyz;
    }    
}