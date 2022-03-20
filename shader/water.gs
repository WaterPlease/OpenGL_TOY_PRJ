#version 430 core

#define M_PI 3.1415926535897932384626433832795

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform float landSize;
uniform float waveLength;
uniform float steepness;
uniform float waterLevel;

out VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 getUV(vec3 v){
    return (v.xz+vec2(landSize*0.5))/landSize;
}

mat4x3 trochoidal(vec3 v, vec2 dir, float steep, float wavelength){
    
	vec3 p = v;

	float k = 2 * M_PI / wavelength;
    float c = sqrt(9.8 / k);
	float f = k * (dot(dir,p.xz) - c * time);
	float a = steep / k;
	
    vec2 d = normalize(dir);

    p.x += d.x * (a * cos(f));
	p.y = a * sin(f);
	p.z += d.y * (a * cos(f));

	vec3 tangent = vec3(
	    1 - d.x * d.x * (steep * sin(f)),
	    d.x * (steep * cos(f)),
	    -d.x * d.y * (steep * sin(f))
    );
    vec3 binormal = vec3(
	    -d.x * d.y * (steep * sin(f)),
	    d.y * (steep * cos(f)),
	    1 - d.y * d.y * (steep * sin(f))
    );
    vec3 normal = normalize(cross(binormal, tangent));
    binormal = normalize(binormal);
    tangent = normalize(tangent);

    mat4x3 res;
    res[0] = p;
    res[1] = tangent;
    res[2] = binormal;
    res[3] = normal;
    return res;
}

mat4x3 multi_trochoidal(vec3 v){
    mat4x3 res = trochoidal(v,normalize(vec2(1.0,-1.0)),steepness,waveLength);
    res    +=     trochoidal(v,vec2(1.0,0.0),steepness*0.5,waveLength*0.5);
    res    +=     trochoidal(v,normalize(vec2(0.2,0.8)),steepness*0.5,waveLength*0.5);
    res[0] *= 0.333;
    res[1] = normalize(res[1]);
    res[2] = normalize(res[2]);
    res[3] = normalize(res[3]);
    return res;
}

void main() {
    vec3 v0 = gl_in[0].gl_Position.xyz;
    vec3 v1 = gl_in[1].gl_Position.xyz;
    vec3 v2 = gl_in[2].gl_Position.xyz;

    mat4x3 res0 = multi_trochoidal(v0);
    mat4x3 res1 = multi_trochoidal(v1);
    mat4x3 res2 = multi_trochoidal(v2);

    vs_out.FragPos = res0[0]+vec3(0.0,waterLevel,0.0);
    vs_out.TexCoords = getUV(res0[0]);
    vs_out.TBN = mat3(res0[1],res0[2],res0[3]);
    gl_Position = projection * view * vec4(vs_out.FragPos,1.0);
    EmitVertex();
    vs_out.FragPos = res1[0]+vec3(0.0,waterLevel,0.0);
    vs_out.TexCoords = getUV(res1[0]);
    vs_out.TBN = mat3(res1[1],res1[2],res1[3]);
    gl_Position = projection * view * vec4(vs_out.FragPos,1.0);
    EmitVertex();
    vs_out.FragPos = res2[0]+vec3(0.0,waterLevel,0.0);
    vs_out.TexCoords = getUV(res2[0]);
    vs_out.TBN = mat3(res2[1],res2[2],res2[3]);
    gl_Position = projection * view * vec4(vs_out.FragPos,1.0);
    EmitVertex();

    EndPrimitive();
}  