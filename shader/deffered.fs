#version 430 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D texture_shadow;

uniform vec3 sunDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform mat4 lightSpaceMat;
uniform float shadowBlurJitter;
uniform float shadowBlurArea;


#define EPS 0.001
#define M_PI 3.1415926535897932384626433832795
#define SHADOW_SAMPLE 64
#define SHADOW_SAMPLE_HALF 32
#define SHADOW_SAMPLE_INV (1.0/SHADOW_SAMPLE)
#define SHADOW_SAMPLE_SQRT 8

// SHADOW MAPPING
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

void main(){

    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    vec3 lighting;
    if(length(Normal)<EPS){
        lighting = Albedo * lightColor;
    }else{
        seed = length(FragPos);
        lighting = Albedo * 0.1; // hard-coded ambient component
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 lightDir = normalize(sunDir);
        vec3 halfWay = normalize(viewDir+lightDir);

        vec3 diffuse = (max(dot(Normal, lightDir), 0.0) * Albedo * lightColor + lightColor * max(pow(dot(Normal,halfWay),40.0),0.0)*Specular) * (1.0-calcShadow(lightSpaceMat * vec4(FragPos,1.0),Normal,lightDir));
        lighting += diffuse;
    }

    
    FragColor = vec4(lighting, 1.0);
}