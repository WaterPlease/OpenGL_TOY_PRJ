#version 430 core

#define EPS 0.001

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D image_position;
uniform sampler2D image_normal;
uniform sampler2D image_color;

uniform vec3 camPos;
uniform vec3 camFront;
uniform mat4 Prjmat;

uniform float maxDistance;
uniform float resolution;
uniform int   steps;
uniform float thickness;

float rayStep = 0.2f;
int iterationCount = 100;
float distanceBias = 0.05f;
bool enableSSR = true;
int sampleCount = 4;
bool isSamplingEnabled = false;
bool isExponentialStepEnabled = false;
bool isAdaptiveStepEnabled = true;
bool isBinarySearchEnabled = true;
bool debugDraw = false;

float LOD1 = 30.0f;

float random (vec2 uv) {
	return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453123); //simple random function
}

vec2 generateProjectedPosition(vec3 pos){
	vec4 samplePosition = Prjmat * vec4(pos, 1.f);
	samplePosition.xy = (samplePosition.xy / samplePosition.w) * 0.5 + 0.5;
	return samplePosition.xy;
}

vec3 SSR(vec3 position, vec3 reflection) {
	vec3 step = rayStep * reflection;
	vec3 marchingPosition = position + step;
	float delta;
	float depthFromScreen;
	vec2 screenPosition;
	
	int i = 0;
	for (; i < iterationCount; i++) {
		screenPosition = generateProjectedPosition(marchingPosition);
		depthFromScreen = abs(texture(image_position,screenPosition).z);
		delta = abs(marchingPosition.z) - depthFromScreen;
		if (abs(delta) < distanceBias) {
            float m = clamp(distance(marchingPosition,position)/maxDistance,0.0,1.0);
            float clip = clamp(length(screenPosition-0.5)*1.5 ,0.0,1.0);

            float w = smoothstep(0.0,1.0,clamp((m+clip)*0.5,0.0,1.0));
			return mix(texture(image_color, screenPosition).rgb,texture(image_color, TexCoords).rgb,w);
		}
		if (isBinarySearchEnabled && delta > 0) {
			break;
		}
		if (isAdaptiveStepEnabled){
			float directionSign = sign(abs(marchingPosition.z) - depthFromScreen);
			//this is sort of adapting step, should prevent lining reflection by doing sort of iterative converging
			//some implementation doing it by binary search, but I found this idea more cheaty and way easier to implement
			step = step * (1.0 - rayStep * max(directionSign, 0.0));
			marchingPosition += step * (-directionSign);
		}
		else {
			marchingPosition += step;
		}
		if (isExponentialStepEnabled){
			step *= 1.05;
		}
    }
	if(isBinarySearchEnabled){
		for(; i < iterationCount; i++){
			
			step *= 0.5;
			marchingPosition = marchingPosition - step * sign(delta);
			
			screenPosition = generateProjectedPosition(marchingPosition);
			depthFromScreen = abs(texture(image_position,screenPosition).z);
			delta = abs(marchingPosition.z) - depthFromScreen;
			
			if (abs(delta) < distanceBias) {
                float m = clamp(distance(marchingPosition,position)/maxDistance,0.0,1.0);
                float clip = clamp(length(screenPosition*vec2(1.3,1.0)-0.5)*1.5 ,0.0,1.0);
                float w = smoothstep(0.0,1.0,clamp(((m+clip)-0.5)*2.0,0.0,1.0));
                return mix(texture(image_color, screenPosition).rgb,texture(image_color, TexCoords).rgb,w);
			}
		}
	}
	
    return vec3(0.0);
}

void main(){
	vec3 position = texture(image_position,TexCoords).xyz;
	vec4 normal = vec4(texture(image_normal, TexCoords).xyz, 0.0);
	float metallic = texture(image_position,TexCoords).a;
	if (!enableSSR || metallic < 0.01 || length(position)>LOD1 ||dot(camFront,normalize(position))<0.4) {
		FragColor = texture(image_color, TexCoords);
	}else {
		vec3 reflectionDirection = normalize(reflect(position, normalize(normal.xyz)));
        if(dot(reflectionDirection,position)>0)
            FragColor = vec4(SSR(position, normalize(reflectionDirection)), 1.0f);
        if (FragColor.xyz == vec3(0.0f)){
            FragColor = texture(image_color, TexCoords);
        }
	}
}