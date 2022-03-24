#version 430 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

#define EPS 0.001
#define M_PI 3.1415926535897932384626433832795
#define SHADOW_SAMPLE 64
#define SHADOW_SAMPLE_HALF 32
#define SHADOW_SAMPLE_INV (1.0/SHADOW_SAMPLE)
#define SHADOW_SAMPLE_SQRT 8

uniform sampler2D texture_diffuse_grass;
uniform sampler2D texture_diffuse_rock;

uniform sampler2D texture_normal_grass;
uniform sampler2D texture_normal_rock;

uniform sampler2D texture_spec_grass;
uniform sampler2D texture_spec_rock;

uniform sampler2D texture_ao_grass;
uniform sampler2D texture_ao_rock;

uniform sampler2D texture_shadow;

uniform vec3 lightDir;
uniform vec3 camPos;
uniform float max_height;
uniform float Kd;
uniform float Ks;
uniform float Ns;
uniform int shadowFactor;
uniform float uvFactorRock;
uniform float uvFactorGrass;
uniform float grassCrit;

uniform float shadowBlurJitter;
uniform float shadowBlurArea;

out vec4 FragColor;

in FGINFO
{
    vec2 uv;
    mat3 TBN;
    vec3 pos;
    vec4 lightSpacePos;
}fginfo;
uniform mat4 view;

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

float shadowStr;
float directional_lighting(sampler2D normal_map,sampler2D spec_map,sampler2D ao_map,float uvFactor,vec3 lDir,vec3 viewDir,float amb,float kd, float ks, float ns){
    vec3 normal = texture(normal_map,uvFactor*fginfo.uv).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(fginfo.TBN * normal);

    float spec = texture(spec_map,uvFactor*fginfo.uv).r;
    
    float diffuse = max(dot(normal, lDir),0.0) * kd;
    
    vec3 H = normalize(lDir + viewDir);
    float specular = pow(max(dot(normal,H),0),ns) * spec;

    if(shadowStr<-EPS){
        shadowStr = (1.0-calcShadow(fginfo.lightSpacePos,normal,lDir));
    }
    return amb+(diffuse + specular)*texture(ao_map,uvFactor*fginfo.uv).r*shadowStr;
}

void main()
{   
    seed = dot(fginfo.pos,fginfo.pos);

    float blend = 0.1;
    float rockStart = grassCrit;
        
    vec3 upVec = vec3(0.0,1.0,0.0);
    float planeWeight = clamp(dot(upVec,fginfo.TBN*vec3(0.0,0.0,1.0)),0.0,1.0);

    float rockWeight = smoothstep(0.0,1.0,(rockStart-planeWeight)/blend);

    vec3 normal_grass = texture(texture_normal_grass,uvFactorGrass*fginfo.uv).rgb;
    normal_grass = normal_grass * 2.0 - 1.0;
    normal_grass = normalize(fginfo.TBN * normal_grass);
    vec3 normal_rock = texture(texture_normal_rock,uvFactorRock*fginfo.uv).rgb;
    normal_rock = normal_rock * 2.0 - 1.0;
    normal_rock = normalize(fginfo.TBN * normal_rock);
    vec3 normal = mix(normal_grass, normal_rock,rockWeight);

    vec3 color_grass = texture(texture_diffuse_grass,uvFactorGrass*fginfo.uv).rgb;
    vec3 color_rock = texture(texture_diffuse_rock,uvFactorRock*fginfo.uv).rgb;
    vec3 albedo = mix(color_grass, color_rock,rockWeight);

    float spec_grass = texture(texture_spec_grass,uvFactorGrass*fginfo.uv).r;
    float spec_rock  = texture(texture_spec_rock,uvFactorRock*fginfo.uv).r;
    float spec = mix(spec_grass, spec_rock,rockWeight);
   
    
    gPosition.xyz = (view*vec4(fginfo.pos,1.0)).xyz;
    gPosition.w = 0.0;

    gNormal.xyz = normalize((view*vec4(normalize(normal),0.0)).xyz);
    gNormal.w;
    gAlbedoSpec.rgb = albedo.rgb;
    gAlbedoSpec.a = spec;

}