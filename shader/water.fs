#version 430 core

layout (location = 0) out vec4 gPositionMetal;
layout (location = 1) out vec4 gNormalRough;
layout (location = 2) out vec4 gAlbedoAO;

#define waterColor (vec3(0.,0.624,0.882))

#define EPS 0.001
#define M_PI 3.1415926535897932384626433832795

out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec4 lightSpacePos;
} fs_in;
uniform mat4 view;
uniform mat4 invViewMat;

uniform sampler3D texture_normal;
uniform samplerCube texture_skybox;
uniform sampler2D texture_shadow;

uniform sampler2D texture_position;
uniform sampler2D texture_color;

uniform float time;
uniform vec3 lightDir;
uniform vec3 camPos;
uniform float uvFactorWater;
uniform float waterTimeFactor;
uniform float res_x;
uniform float res_y;
uniform float waterLevel;

uniform float transparency;

void main()
{   
    vec3 normal = texture(texture_normal, vec3(uvFactorWater * fs_in.TexCoords,time*waterTimeFactor)).rgb;
    normal = 2.0*normal-vec3(1.0);
    normal = normal * 0.3 + 0.7 * vec3(0.0,0.0,1.0);
    //normal = vec3(0.0,1.0,0.0);
    normal = normalize(fs_in.TBN * normal);

    vec2 uv = gl_FragCoord.xy/vec2(res_x,res_y);

    vec4 ground = invViewMat * vec4(texture(texture_position,uv).xyz,1.0);
    //float depth = abs(distance(ground.xyz,camPos)-distance(fs_in.FragPos,camPos));
    float depth = distance(ground.xyz,fs_in.FragPos);
    float foamWeight = smoothstep(0.0,1.0,clamp(1-(depth-0.05)*8,0.0,1.0));

    gPositionMetal.xyz = (view*vec4(fs_in.FragPos,1.0)).xyz;
    gPositionMetal.w = min(1-foamWeight,0.3);
    gNormalRough.xyz = normalize((view*vec4(normal,0.0)).xyz);
    gNormalRough.w = 0.0;
    //gAlbedoAO.rgb = mix(waterColor,vec3(1.0),foamWeight);
    gAlbedoAO.rgb = mix(mix(waterColor,texture(texture_color,uv).rgb,exp(-depth/transparency)),vec3(1.0),foamWeight);
    gAlbedoAO.a   = 1.0;
}