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

uniform vec3 lightDir;
uniform float max_height;
uniform float Kd;
uniform float Ks;
uniform float Ns;

out vec4 FragColor;

in FGINFO
{
    vec2 uv;
    vec3 TBNViewDir;
    vec3 TBNLightDir;
    vec3 normal;
    vec3 pos;
}fginfo;

float uvFactorRock = 300.0;
float uvFactorGrass = 300.0;

float directional_lighting(sampler2D normal_map,sampler2D spec_map,sampler2D ao_map,float uvFactor,float kd, float ks, float ns){
    vec3 normal = texture(normal_map,uvFactor*fginfo.uv).rgb;
    float spec = texture(spec_map,uvFactor*fginfo.uv).r;
    normal = normalize(normal * 2.0 - 1.0);
    
    float ambient = 0.07;//max(1.0-kd-ks,0.0);
    float diffuse = max(dot(normal, fginfo.TBNLightDir),0.0) * kd;
    
    vec3 H = normalize(fginfo.TBNLightDir + fginfo.TBNViewDir);
    float specular = pow(max(dot(normal,H),0),ns) * spec * max(dot(lightDir,vec3(0.0,1.0,0.0)),0);

    return (ambient + diffuse + specular)*texture(ao_map,uvFactor*fginfo.uv).r;
}

void main()
{   
    float blend = 0.1;
    float rockStart = 0.99;
        
    vec3 upVec = vec3(0.0,1.0,0.0);
    float planeWeight = clamp(dot(upVec,fginfo.normal),0.0,1.0);
    /*
    if(planeWeight > rockStart){
        lighting = vec3(directional_lighting(texture_normal_grass,texture_spec_grass,texture_ao_grass,uvFactorGrass,Kd,Ks,Ns));
        albedo = texture(texture_diffuse_grass,uvFactorGrass*fginfo.uv).xyz;
    }else if(planeWeight>(rockStart-blend)){

        vec3 result_rock = 
    }else{
        lighting = vec3(directional_lighting(texture_normal_rock,texture_spec_rock,texture_ao_rock,uvFactorRock,Kd,Ks,Ns));
        albedo = texture(texture_diffuse_rock,uvFactorRock*fginfo.uv).xyz;
    }*/
    
    vec3 lighting_rock = vec3(directional_lighting(texture_normal_rock,texture_spec_rock,texture_ao_rock,uvFactorRock,Kd,Ks,Ns));
    vec3 color_rock = texture(texture_diffuse_rock,uvFactorRock*fginfo.uv).xyz;

    vec3 lighting_grass = vec3(directional_lighting(texture_normal_grass,texture_spec_grass,texture_ao_grass,uvFactorGrass,Kd,Ks,Ns));
    vec3 color_grass = texture(texture_diffuse_grass,uvFactorGrass*fginfo.uv).xyz;

    vec3 result_rock = color_rock * lighting_rock;
    vec3 result_grass= color_grass * lighting_grass;

    float rockWeight = smoothstep(0.0,1.0,(rockStart-planeWeight)/blend);

    vec3 result = mix(result_grass, result_rock,rockWeight);

    //vec3 result = clamp(albedo*lighting,0.0,1.0);

    FragColor = vec4(result,1.0);//vec4((fginfo.normal+vec3(1.0))/2.0,1.0);
}