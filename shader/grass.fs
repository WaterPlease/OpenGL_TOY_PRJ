#version 430 core

#define GRASS_COLOR vec3(0.0,0.7,0.0)

#define EPS 0.001
#define M_PI 3.1415926535897932384626433832795
#define SHADOW_SAMPLE 64
#define SHADOW_SAMPLE_HALF 32
#define SHADOW_SAMPLE_INV (1.0/SHADOW_SAMPLE)
#define SHADOW_SAMPLE_SQRT 8

in vec2 texCoords;
in vec4 lightSpacePos;
in vec3 fNormal;
in vec3 viewDir;

uniform vec3 lightDir;
uniform int shadowFactor;

uniform sampler2D texture_shadow;
uniform sampler2D texture_grass;

out vec4 FragColor;

vec3 cl;

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
float calcShadow(vec4 fragLightSpace,vec3 lDir){
    vec3 pos = fragLightSpace.xyz/fragLightSpace.w;
    pos = pos * 0.5 + 0.5;
    
    float currentDepth = pos.z;
    
    float bias = 0.0;// = max(0.05 * (1.0 - dot(normal, lDir)), 0.005);

    float shadow = 0.0;
    if(pos.z>1.0){
        //
    }
    //else if(dot(normal,lDir)<EPS){
    //    shadow = 1.0;
    //}
    else{
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

float directional_lighting(float amb,float kd, float ks, float ns){    
    float ambient = amb;
    float diffuse = max(dot(vec3(0.0,1.0,0.0),lightDir),0.0) * kd;

    return (ambient + (diffuse)*(1.0-calcShadow(lightSpacePos,lightDir)));
}

void main()
{   
    vec4 color = texture(texture_grass,texCoords);
    if(color.a < 0.1     ||
       texCoords.y < 0.3 || 
       texCoords.x<0.1   ||
       texCoords.x>0.95  ||
       abs(dot(viewDir,fNormal))<0.1)
        discard;
    seed = dot(lightSpacePos,lightSpacePos);
    vec3 result = color.rgb * directional_lighting(0.2,0.6,0.0,20.0);// * min(1.0,max(0.2,1.2*dot(lightDir,vec3(0.0,1.0,0.0))));

    FragColor = vec4(result,1.0);//vec4((fginfo.normal+vec3(1.0))/2.0,1.0);
    
}