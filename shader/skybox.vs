#version 430 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;
out vec3 fragPos;

uniform mat4 view;
uniform mat4 projection;

void main(){
	TexCoords = aPos;
	vec4 pos;
	fragPos = (view*vec4(aPos,0.0)).xyz*100000.0;
	pos = projection * view * vec4(aPos,1.0);
	gl_Position = pos.xyww;
}