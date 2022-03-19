#version 430 core

#define M_PI 3.1415926535897932384626433832795

layout (triangles) in;
layout (triangle_strip, max_vertices = 5) out;

uniform float grassWidth;
uniform float grassLean;
uniform float grassHeight;
uniform vec3 camPos;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 camFront;
uniform float cosHalfDiag;
uniform float waterLevel;

out vec3 fNormal;
out vec3 viewDir;
out float green_weight;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float lod1 = 120.0;
float lod2 = 90.0;

void main() {
    vec4 center = vec4(((gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position)/3.0).xyz,0.0);
    vec3 upVec  = vec3(0,1.0,0);
    vec3 normal = normalize(cross(gl_in[1].gl_Position.xyz-gl_in[0].gl_Position.xyz,gl_in[2].gl_Position.xyz-gl_in[0].gl_Position.xyz));
    float planeWeight = dot(upVec,normal);
    
	float testVal_3d = dot(normalize(center.xyz-camPos),camFront);
    if(testVal_3d < cosHalfDiag || center.y<waterLevel){
            
    }else if(planeWeight > 0.998){
        if(0<1){//distance(center.xz,camPos.xz)>60.0){
            float rad = 2*M_PI*rand(center.xz);
            float r1 = rand(vec2(rad));
            float r2 = rand(vec2(r1));
            float r3 = rand(vec2(r2));
            float r4 = rand(vec2(r3));
            float r5 = rand(vec2(r4));
            r3 = r3 * 0.2+0.9;
            r4 = r4 * 0.2+0.9;
            r5 = r5 * 0.2+0.9;
            float dist = distance(center.xz,camPos.xz);//max(distance(center.xz,camPos.xz),distance(center.xyz,camPos.xyz));
            if(dist>lod1){
                r3 = r3 * 16.0;
            }else if(dist>lod2){
                r3 = r3 * 4.0;
            }

            // grass generation
            vec4 vertices[5];
            vertices[0] = vec4(-0.5,0.0,0.0,1.0);
            vertices[1] = vec4(0.5,0.0,0.0,1.0);
            vertices[2] = vec4(-0.25,0.5,0.5,1.0);
            vertices[3] = vec4(0.25,0.5,0.5,1.0);
            vertices[4] = vec4(0.0,1.0,1.0,1.0);
            vec4 grassTransform = vec4(grassWidth*r3,grassHeight*r4,grassLean*r5,1.0);

            // grass rotation
            mat4 rot;
            float cosVal = cos(rad);
            float sinVal = sin(rad);
            rot[0] = vec4(cosVal,0.0,-sinVal,0.0);
            rot[1] = vec4(0.0,1.0,0.0,0.0);
            rot[2] = vec4(sinVal,0.0,cosVal,0.0);
            rot[3] = vec4(0.0,0.0,0.0,1.0);
            center = center + vec4(r1-0.5,-0.01,r2-0.5,0.0);
            green_weight = rand(vec2(r2))*0.6+0.4;

            vertices[0] = rot * (vertices[0] * grassTransform) + center;
            vertices[1] = rot * (vertices[1] * grassTransform) + center;
            vertices[2] = rot * (vertices[2] * grassTransform) + center;
            vertices[3] = rot * (vertices[3] * grassTransform) + center;
            vertices[4] = rot * (vertices[4] * grassTransform) + center;
            fNormal = vec4(cross(vertices[1].xyz-vertices[0].xyz,vertices[2].xyz-vertices[0].xyz),1.0).xyz;
            fNormal = -normalize(fNormal);

            gl_Position = projection * view * vertices[0];
            viewDir = normalize(vertices[0].xyz - camPos);
            EmitVertex();
            gl_Position = projection * view * vertices[1];
            viewDir = normalize(vertices[1].xyz - camPos);
            EmitVertex();
            gl_Position = projection * view * vertices[2];
            viewDir = normalize(vertices[2].xyz - camPos);
            EmitVertex();
            gl_Position = projection * view * vertices[3];
            viewDir = normalize(vertices[3].xyz - camPos);
            EmitVertex();
            gl_Position = projection * view * vertices[4];
            viewDir = normalize(vertices[4].xyz - camPos);
            EmitVertex();
        }
    }
    EndPrimitive();
}  