#version 430 core

#define GRASS_COLOR vec3(0.0,1.0,0.0)
#define ROCK_COLOR vec3(0.33,0.25,0.14)

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

out vec4 FragColor;

in FGINFO
{
    vec2 uv;
    mat3 TBN;
    vec3 pos;
    vec4 lightSpacePos;
}fginfo;

float uvFactorRock = 300.0;
float uvFactorGrass = 100.0;

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

float directional_lighting(sampler2D normal_map,sampler2D spec_map,sampler2D ao_map,float uvFactor,vec3 lDir,vec3 viewDir,float amb,float kd, float ks, float ns){
    vec3 normal = texture(normal_map,uvFactor*fginfo.uv).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = fginfo.TBN * normal;

    float spec = texture(spec_map,uvFactor*fginfo.uv).r;
    
    float diffuse = max(dot(normal, lDir),0.0) * kd;
    
    vec3 H = normalize(lDir + viewDir);
    float specular = pow(max(dot(normal,H),0),ns) * spec * ks;

    return amb+(diffuse + specular)*texture(ao_map,uvFactor*fginfo.uv).r*(1.0-calcShadow(fginfo.lightSpacePos,normal,lDir));
}

void main()
{   
    float blend = 0.1;
    float rockStart = 0.99;
        
    vec3 upVec = vec3(0.0,1.0,0.0);
    float planeWeight = clamp(dot(upVec,fginfo.TBN*vec3(0.0,0.0,1.0)),0.0,1.0);
    
    vec3 lighting_rock = vec3(directional_lighting(texture_normal_rock,texture_spec_rock,texture_ao_rock,uvFactorRock
    ,lightDir,camPos-fginfo.pos,0.2,Kd,Ks,Ns));
    vec3 color_rock = texture(texture_diffuse_rock,uvFactorRock*fginfo.uv).xyz;

    vec3 lighting_grass = vec3(directional_lighting(texture_normal_grass,texture_spec_grass,texture_ao_grass,uvFactorGrass
    ,lightDir,camPos-fginfo.pos,0.2,Kd,Ks,Ns));
    vec3 color_grass = texture(texture_diffuse_grass,uvFactorGrass*fginfo.uv).xyz;

    vec3 result_rock = color_rock * lighting_rock;
    vec3 result_grass= color_grass * lighting_grass;

    float rockWeight = smoothstep(0.0,1.0,(rockStart-planeWeight)/blend);

    vec3 result = mix(result_grass, result_rock,rockWeight);

    FragColor = vec4(result,1.0);//vec4(fginfo.TBN*vec3(0.0,0.0,1.0),1.0);
}