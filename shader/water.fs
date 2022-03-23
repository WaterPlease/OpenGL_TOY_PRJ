#version 430 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

#define waterColor vec3(0.,0.624,0.882)

#define EPS 0.001
#define M_PI 3.1415926535897932384626433832795
#define SHADOW_SAMPLE 64
#define SHADOW_SAMPLE_HALF 32
#define SHADOW_SAMPLE_INV (1.0/SHADOW_SAMPLE)
#define SHADOW_SAMPLE_SQRT 8

out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec4 lightSpacePos;
} fs_in;

uniform sampler3D texture_normal;
uniform samplerCube texture_skybox;
uniform sampler2D texture_shadow;

uniform float time;
uniform vec3 lightDir;
uniform vec3 camPos;
uniform float uvFactorWater;
uniform float waterTimeFactor;
uniform int shadowFactor;

uniform float shadowBlurJitter;
uniform float shadowBlurArea;

float seed;
float rand(){
    seed = fract(sin(dot(vec2(seed), vec2(12.9898, 78.233))) * 43758.5453);
    return seed;
}
vec4 getJitOffset(int u, int v){
    vec4 offset;
    offset.xy = vec2(u,v) * (1.0/(SHADOW_SAMPLE_SQRT));
    offset.zw = offset.xy;
    offset.z = 1.0-offset.x;
    return offset + (vec4(rand(),rand(),rand(),rand())-0.5)*shadowBlurJitter;
}
vec2 dartSampleSpace(vec2 jit){
    return sqrt(abs(jit.y))*vec2(sin(jit.x*2.0*M_PI),cos(jit.x*2.0*M_PI));
}
bool fast;
float calcShadow(vec4 fragLightSpace,vec3 normal,vec3 lDir){
    vec3 pos = fragLightSpace.xyz/fragLightSpace.w;
    pos = pos * 0.5 + 0.5;
    
    float currentDepth = pos.z;
    
    float bias = max(0.05 * (1.0 - dot(normal, lDir)), 0.005);

    float shadow = 0.0;
    if(pos.z>1.0){
        //
    }else if(dot(normal,lDir)<EPS){
        shadow = 1.0;
    }else{
        float pcfDepth;
        vec2 texelSize = shadowBlurArea*sqrt(SHADOW_SAMPLE_SQRT) / textureSize(texture_shadow,0);
        for(int i=0;i<4;i++){
            vec4 jit = getJitOffset(7,i);
            pcfDepth = texture(texture_shadow,pos.xy+dartSampleSpace(jit.xy)*texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth ? (1.0/8.0):0.0;
            pcfDepth = texture(texture_shadow,pos.xy+dartSampleSpace(jit.zw)*texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth ? (1.0/8.0):0.0;
        }
        if(shadow*(1-shadow)>EPS){
            fast = false;
            shadow *= 1.0/8;
            for(int i=0;i<SHADOW_SAMPLE_SQRT-1;i++){
                for(int j=0;j<4;j++){
                    vec4 jit = getJitOffset(i,j);
                    pcfDepth = texture(texture_shadow,pos.xy+dartSampleSpace(jit.xy)*texelSize).r;
                    shadow += (currentDepth - bias) > pcfDepth ? (SHADOW_SAMPLE_INV):0.0;
                    pcfDepth = texture(texture_shadow,pos.xy+dartSampleSpace(jit.zw)*texelSize).r;
                    shadow += (currentDepth - bias) > pcfDepth ? (SHADOW_SAMPLE_INV):0.0;
                }
            }
        }
        //pcfDepth = texture(texture_shadow,pos.xy).r;
        //shadow *= (currentDepth - bias) > pcfDepth ? 1.0:0.0;
    }
    return smoothstep(0.0,1.0,shadow);
}

float directional_lighting(vec3 normal,float amb,float kd, float ks, float ns){    
    float ambient = max(1.0-kd-ks,amb);
    float diffuse = max(dot(normal,lightDir),0.0) * kd;
    
    //vec3 H = normalize(lightDir + (camPos-fs_in.FragPos));
    //float specular = pow(max(dot(normal,H),0),ns) * ks;
    vec3 viewDir = normalize(camPos-fs_in.FragPos);
    //vec3 reflectDir = reflect(-lightDir, normal);
    //float specular = pow(max(dot(viewDir, reflectDir), 0.0), 32)*ks;
    vec3 halfWayDir = normalize(lightDir+viewDir);
    float specular = pow(max(dot(normal, halfWayDir), 0.0), 32)*ks;

    return ambient + (diffuse + specular) * (1.0 - calcShadow(fs_in.lightSpacePos,normal,lightDir));
}

void main()
{   
    vec3 normal = texture(texture_normal, vec3(uvFactorWater * fs_in.TexCoords,time*waterTimeFactor)).rgb;
    normal = 2.0*normal-vec3(1.0);
    normal = normalize(fs_in.TBN * normal);

    gPosition = fs_in.FragPos;
    gNormal = normal;
    gAlbedoSpec.rgb = waterColor;
    gAlbedoSpec.a   = 1.0;
}