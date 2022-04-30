#version 430 core

layout(std430, binding = 3) volatile buffer flyInfo {
	float pos [32*32*5];
}flyinfo;

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPositionMetal;
uniform sampler2D gNormalRough;
uniform sampler2D gAlbedoAO;
uniform sampler2D texture_shadow;

uniform vec3 sunDir;
uniform vec3 lightColor;
uniform float sunStrength;
uniform vec3 viewPos;
uniform mat4 inverseViewMat;
uniform mat4 view;
uniform mat4 lightSpaceMat;
uniform float shadowBlurJitter;
uniform float shadowBlurArea;
uniform float landSize;
uniform bool drawFireflies;


#define EPS 0.001
#define M_PI 3.1415926535897932384626433832795
#define SHADOW_SAMPLE 64
#define SHADOW_SAMPLE_HALF 32
#define SHADOW_SAMPLE_INV (1.0/SHADOW_SAMPLE)
#define SHADOW_SAMPLE_SQRT 8

#define NUM_POINT_LIGHT 1024




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
    
    float bias = max(0.005 * (1.0 - dot(normal, lDir)), 0.005);

    float shadow = 0.0;
    if(pos.z>1.0){
        //
    }else if(dot(normal,lDir)<0.0){
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









// PBR RENDERING

vec3 fresnelSchlick(float cosTheta, vec3 F0){
    return F0 + (1-F0) * pow(clamp(1.0-cosTheta,0.0,1.0),5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness){
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N,H),0.0);
    float NdotH2 = NdotH*NdotH;

    float divNum = NdotH2*(a2-1.0)+1.0;
    divNum = M_PI * divNum * divNum;

    return a2/divNum;
}
float GeometrySchlickGGX(float NdotV, float roughness){
    float r = (roughness+1.0);
    float k = (r*r) / 8.0;

    float divNum = NdotV * (1.0-k) + k;

    return NdotV/divNum;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness){
    float NdotV = max(dot(N,V),0.0);
    float NdotL = max(dot(N,L),0.0);

    float ggx2 = GeometrySchlickGGX(NdotV,roughness);
    float ggx1 = GeometrySchlickGGX(NdotL,roughness);

    return ggx1*ggx2;
}

vec3 pbr_sun_lighting(vec4 lightSpacePos,vec3 lightColor,vec3 L,vec3 N, vec3 V, vec3 albedo, float metalic, float roughness,float ao){
    vec3 Lo;
    vec3 H = normalize(N+V);
    vec3 radiance =  lightColor;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0,albedo,metalic);

    vec3 F = fresnelSchlick(max(dot(H,V),0.0),F0);

    float NDF = DistributionGGX(N,H,roughness);
    float G   = GeometrySmith(N,V,L,roughness);

    vec3 numerator = NDF*G*F;
    float denominator = 4.0 * max(dot(N,V),0.0) * max(dot(N,L),0.0);
    denominator = max(denominator,0.0001);

    vec3 specular = numerator/denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0)-kS;
    kD *= (1.0-metalic);

    float NdotL = max(dot(N,L),0.0);

    Lo = (kD * albedo / M_PI + specular ) * radiance * NdotL * (1.0-calcShadow(lightSpacePos,N,L));
    return Lo + vec3(0.03)*albedo*ao;
}


vec3 pbr_sun_point(float dist,vec3 lightColor,vec3 L,vec3 N, vec3 V, vec3 albedo, float metalic, float roughness,float ao){
    vec3 Lo;
    vec3 H = normalize(N+V);
    vec3 radiance =  lightColor / (1.0+dist*dist);
    //vec3 radiance =  lightColor / ((dist*5.0)*(dist*5.0)+1) * pow(max(1-dist,0.0),42);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0,albedo,metalic);

    vec3 F = fresnelSchlick(max(dot(H,V),0.0),F0);

    float NDF = DistributionGGX(N,H,roughness);
    float G   = GeometrySmith(N,V,L,roughness);

    vec3 numerator = NDF*G*F;
    float denominator = 4.0 * max(dot(N,V),0.0) * max(dot(N,L),0.0);
    denominator = max(denominator,0.0001);

    vec3 specular = numerator/denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0)-kS;
    kD *= (1.0-metalic);

    float NdotL = max(dot(N,L),0.0);

    Lo = (kD * albedo / M_PI + specular ) * radiance * NdotL;
    return Lo;// + vec3(0.03)*albedo*ao;
}


// MAIN
void main(){
    seed = (gl_FragCoord.x/1980)*(gl_FragCoord.y/1080);

    vec4 gPositionMetal = texture(gPositionMetal, TexCoords);
    vec4 gAlbedoAO = texture(gAlbedoAO, TexCoords);
    vec4 gNormalRough = texture(gNormalRough, TexCoords);

    vec3 FragPos = gPositionMetal.rgb;
    float metalic = gPositionMetal.a;

    float roughness = gNormalRough.a;
    
    vec3 Albedo = gAlbedoAO.rgb;
    float AO = gAlbedoAO.a;
    
    vec3 lighting = vec3(0.0);
    vec4 lightSpacePos = lightSpaceMat*(inverseViewMat*vec4(FragPos,1.0));

    /*
    */
    vec3 Normal = gNormalRough.rgb;
    vec3 globalFragPos = (inverseViewMat * vec4(FragPos,1.0)).xyz;
    if(length(Normal)<EPS){
        if(AO<0.0)
            lighting += Albedo*lightColor;
        else
            lighting += Albedo;
    }else{
        Normal = normalize(Normal);
        lighting += pbr_sun_lighting(lightSpacePos,lightColor*sunStrength,normalize(sunDir),Normal,normalize(-FragPos),Albedo,metalic,roughness,AO);

        for(int i=0;i<NUM_POINT_LIGHT;i++){
            vec3 lPos = vec3(flyinfo.pos[5*i+0],flyinfo.pos[5*i+1],flyinfo.pos[5*i+2]);
            float dist = distance(globalFragPos,lPos);
            if(dist<2.0){
                vec3 viewLPos = (view * vec4(lPos,1.0)).xyz;
                vec3 color = vec3(0.2);
                color[uint(flyinfo.pos[5*i+4])] = 1.2;
                lighting += pbr_sun_point(dist,color*flyinfo.pos[5*i+3],normalize(viewLPos - FragPos),Normal,normalize(-FragPos),Albedo,metalic,roughness,AO);
            }
        }
    }
    
    FragColor = vec4(lighting, 1.0);
}