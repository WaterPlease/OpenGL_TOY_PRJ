#version 430 core

#define GRASS_COLOR vec3(0.0,0.7,0.0)

in vec3 fNormal;
in vec3 viewDir;
in float green_weight;
in vec4 lightSpacePos;

uniform vec3 lightDir;

uniform sampler2D texture_shadow;

out vec4 FragColor;

vec3 cl;

float calcShadow(vec4 fragLightSpace,vec3 normal,vec3 lDir){
    vec3 pos = fragLightSpace.xyz/fragLightSpace.w;
    pos = pos * 0.5 + 0.5;
    
    float currentDepth = pos.z;
    
    float bias = max(0.05 * (1.0 - dot(normal, lDir)), 0.005);

    float shadow = 0.0;
    float pcfDepth;
    vec2 texelSize = 1.0 / textureSize(texture_shadow,0);
    
    pcfDepth = texture(texture_shadow,pos.xy+vec2(0.0,0.0)*texelSize).r;
    shadow += (currentDepth - bias) > pcfDepth ? 1.0:0.0;
    pcfDepth = texture(texture_shadow,pos.xy+vec2(0.0,1.0)*texelSize).r;
    shadow += (currentDepth - bias) > pcfDepth ? 1.0:0.0;
    pcfDepth = texture(texture_shadow,pos.xy+vec2(0.0,-1.0)*texelSize).r;
    shadow += (currentDepth - bias) > pcfDepth ? 1.0:0.0;
    pcfDepth = texture(texture_shadow,pos.xy+vec2(1.0,0.0)*texelSize).r;
    shadow += (currentDepth - bias) > pcfDepth ? 1.0:0.0;
    pcfDepth = texture(texture_shadow,pos.xy+vec2(-1.0,0.0)*texelSize).r;
    shadow += (currentDepth - bias) > pcfDepth ? 1.0:0.0;
    pcfDepth = texture(texture_shadow,pos.xy+vec2(1.0,1.0)*texelSize).r;
    shadow += (currentDepth - bias) > pcfDepth ? 1.0:0.0;
    pcfDepth = texture(texture_shadow,pos.xy+vec2(-1.0,-1.0)*texelSize).r;
    shadow += (currentDepth - bias) > pcfDepth ? 1.0:0.0;
    pcfDepth = texture(texture_shadow,pos.xy+vec2(1.0,-1.0)*texelSize).r;
    shadow += (currentDepth - bias) > pcfDepth ? 1.0:0.0;
    pcfDepth = texture(texture_shadow,pos.xy+vec2(-1.0,1.0)*texelSize).r;
    shadow += (currentDepth - bias) > pcfDepth ? 1.0:0.0;

    shadow /= 9.0;

    return shadow;
}

float directional_lighting(float amb,float kd, float ks, float ns){    
    float ambient = amb;
    float diffuse = max(dot(fNormal,lightDir),0.0) * kd;
    
    vec3 H = normalize(lightDir + viewDir);
    float specular = pow(max(dot(fNormal,H),0),ns) * ks;

    return (ambient + (diffuse + specular)*(1.0-calcShadow(lightSpacePos,fNormal,lightDir)));
}

void main()
{   
    vec3 result = GRASS_COLOR * directional_lighting(0.2,0.6,0.0,20.0) * green_weight;// * min(1.0,max(0.2,1.2*dot(lightDir,vec3(0.0,1.0,0.0))));

    FragColor = vec4(result,1.0);//vec4((fginfo.normal+vec3(1.0))/2.0,1.0);
    
}