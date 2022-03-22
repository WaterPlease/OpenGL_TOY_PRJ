#version 430 core

#define M_PI 3.1415926535897932384626433832795
#define EPS 0.001

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform float landSize;
uniform float waveLength;
uniform float steepness;
uniform float waterLevel;
uniform mat4 lightSpaceMat;

out VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec4 lightSpacePos;
} vs_out;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 getUV(vec3 v){
    return (v.xz+vec2(landSize*0.5))/landSize;
}

vec3 trochoidal(vec3 v, vec2 dir, float steep, float wavelength){    
	vec3 p = v;

	float k = 2 * M_PI / wavelength;
    float c = sqrt(9.8 / k);
	float f = k * (dot(dir,p.xz) - c * time);
	float a = steep / k;
	
    vec2 d = normalize(dir);

    p.x += d.x * (a * cos(f));
	p.y = a * sin(f);
	p.z += d.y * (a * cos(f));

    return p;
}

vec3 multi_trochoidal(vec3 v){
    vec3 res = trochoidal(v,normalize(vec2(1.0,-1.0)),steepness,waveLength);
    res    +=     trochoidal(v,vec2(1.0,0.0),steepness*0.5,waveLength*0.5);
    res    +=     trochoidal(v,normalize(vec2(0.2,0.8)),steepness*0.5,waveLength*0.5);
    res *= 0.333;
    return res;
}

mat4x3 wave(vec3 v){
    vec3 v0 = multi_trochoidal(v);
    vec3 v1 = multi_trochoidal(v+vec3(EPS,0.0,0.0));
    vec3 v2 = multi_trochoidal(v+vec3(0.0,0.0,EPS));

    vec2 uv0 = getUV(v0);
    vec2 uv1 = getUV(v1);
    vec2 uv2 = getUV(v2);

    vec3 e1 = v1-v0;
    vec3 e2 = v2-v0;

    vec2 duv1 = uv1-uv0;
    vec2 duv2 = uv2-uv0;

    float f = 1.0/(duv1.x*duv2.y-duv1.y*duv2.x);
    vec3 tan = f*(duv2.y*e1-duv1.y*e2);
    vec3 bitan = f*(-duv2.x*e1+duv1.x*e2);
    tan = normalize(tan);
    bitan = normalize(bitan);
    tan = normalize(tan-dot(tan,bitan)*bitan);
    vec3 normal = -normalize(cross(tan,bitan));

    mat4x3 res;
    res[0] = v0;
    res[1] = tan;
    res[2] = bitan;
    res[3] = normal;

    return res;
}

void main() {
    vec3 v0 = gl_in[0].gl_Position.xyz;
    vec3 v1 = gl_in[1].gl_Position.xyz;
    vec3 v2 = gl_in[2].gl_Position.xyz;

    mat4x3 res0 = wave(v0);
    mat4x3 res1 = wave(v1);
    mat4x3 res2 = wave(v2);

    vs_out.FragPos = res0[0]+vec3(0.0,waterLevel,0.0);
    vs_out.TexCoords = getUV(res0[0]);
    vs_out.TBN = mat3(res0[1],res0[2],res0[3]);
    vs_out.lightSpacePos = lightSpaceMat * vec4(vs_out.FragPos,1.0);
    gl_Position = projection * view * vec4(vs_out.FragPos,1.0);
    EmitVertex();
    vs_out.FragPos = res1[0]+vec3(0.0,waterLevel,0.0);
    vs_out.TexCoords = getUV(res1[0]);
    vs_out.TBN = mat3(res1[1],res1[2],res1[3]);
    vs_out.lightSpacePos = lightSpaceMat * vec4(vs_out.FragPos,1.0);
    gl_Position = projection * view * vec4(vs_out.FragPos,1.0);
    EmitVertex();
    vs_out.FragPos = res2[0]+vec3(0.0,waterLevel,0.0);
    vs_out.TexCoords = getUV(res2[0]);
    vs_out.TBN = mat3(res2[1],res2[2],res2[3]);
    vs_out.lightSpacePos = lightSpaceMat * vec4(vs_out.FragPos,1.0);
    gl_Position = projection * view * vec4(vs_out.FragPos,1.0);
    EmitVertex();

    EndPrimitive();
}  