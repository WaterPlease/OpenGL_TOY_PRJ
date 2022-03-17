#version 430 core

#define GRASS_COLOR vec3(0.0,0.7,0.0)

in vec3 fNormal;
in vec3 viewDir;
in float green_weight;

uniform vec3 lightDir;

out vec4 FragColor;

float directional_lighting(float kd, float ks, float ns){    
    float ambient = max(1.0-kd-ks,0.0);
    float diffuse = max(dot(fNormal,lightDir),0.0) * kd;
    
    vec3 H = normalize(lightDir + viewDir);
    float specular = pow(max(dot(fNormal,H),0),ns) * ks;

    return (ambient + diffuse + specular);
}

void main()
{   
    vec3 result = GRASS_COLOR * directional_lighting(0.85,0.0,20.0) * green_weight * (0.1 + dot(lightDir,vec3(0,1.0,0))*0.5);

    FragColor = vec4(result,1.0);//vec4((fginfo.normal+vec3(1.0))/2.0,1.0);
}