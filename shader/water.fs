#version 430 core

#define waterColor vec3(0.,0.624,0.882)

out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform sampler3D texture_normal;
uniform float time;
uniform vec3 lightDir;
uniform vec3 camPos;
uniform float uvFactorWater;
uniform float waterTimeFactor;


float directional_lighting(vec3 normal,float amb,float kd, float ks, float ns){    
    float ambient = max(1.0-kd-ks,amb);
    float diffuse = max(dot(normal,lightDir),0.0) * kd;
    
    vec3 H = normalize(lightDir + (camPos-fs_in.FragPos));
    float specular = pow(max(dot(normal,H),0),ns) * ks;

    return (ambient + diffuse + specular);
}

void main()
{    
    vec3 normal = texture(texture_normal, vec3(uvFactorWater * fs_in.TexCoords,time*waterTimeFactor)).rgb;
    normal = 2.0*normal-vec3(1.0);
    normal = normalize(fs_in.TBN * normal);
    vec3 color = waterColor*directional_lighting(normal,0.1,0.6,0.2,24.0);
    FragColor = vec4(color,1.0);
}