#version 430 core

#define EPS 0.001

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D image_position;
uniform sampler2D image_normal;
uniform sampler2D image_color;
uniform samplerCube texture_skybox;

uniform vec3 camPos;
uniform vec3 camFront;
uniform mat4 Prjmat;
uniform mat4 invViewMat;
uniform vec3 lightColor;

uniform float maxDistance;
uniform float resolution;
uniform int   bin_steps;
uniform int   lin_steps;
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

float LOD1 = 45.0f;

vec4 Raycast(vec3 rayOrigin, vec3 rayDir,sampler2D tex_position){
	// return : (uv coordinates, ray distance, delta depth)
	// 			when ray hit somewhere, ray distance > 0
	//				if not, ray distance < 0

	// parameter init.
	vec2 texSz = textureSize(tex_position,0);

	// start of ray cast
	vec3 rayEnd = rayOrigin + rayDir * maxDistance;

	vec4 screenStart, screenEnd; // ray points in NDC:[0,1]x[0,1]
	
	screenStart = Prjmat * vec4(rayOrigin,1.0);
	screenStart.xyz /= screenStart.w;
	screenStart.xy = screenStart.xy * 0.5 + 0.5;
	screenStart.xy *= texSz;

	screenEnd = Prjmat * vec4(rayEnd,1.0);
	screenEnd.xyz /= screenEnd.w;
	screenEnd.xy = screenEnd.xy * 0.5 + 0.5;
	screenEnd.xy *= texSz;

	vec2 delta = (screenEnd-screenStart).xy; // delta on screen

	if(length(delta) < 2){ // discard when ray is too short on screen
		return vec4(1.0,1.0,-1.0,-1.0);
	}
	float xMajor = (abs(delta.x)>abs(delta.y))? 1:0;

	vec2 inc = ((delta/mix(abs(delta.y),abs(delta.x),xMajor))) * (1/clamp(resolution,0.0,1.0));
	int numLinearIter = min(
							int(mix(abs(delta.y),abs(delta.x),xMajor)),
							lin_steps);
	// ray increasement in screen space that move 1/resoultion pixel.
	
	vec2 searchPos = screenStart.xy;
	vec2 searchPosUV;

	vec4 viewPos;

	float dDepth;

	float search1 = 0;
	float search0 = 0;

	float viewDistance = 0.0;

	int hit1 = -1;
	int hit0 = 0;
	// linear step pass
	int i;
	for(i=0;i<numLinearIter;i++){
		searchPos += inc;
		searchPosUV = searchPos/texSz;
		
		search1 = mix(
						(searchPos.y-screenStart.y)/delta.y,
						(searchPos.x-screenStart.x)/delta.x,
						xMajor
					 );

		viewPos = texture(tex_position,searchPosUV);
		viewDistance = abs((rayOrigin.z*rayEnd.z)/mix(rayEnd.z,rayOrigin.z,search1));

		dDepth = viewDistance-abs(viewPos.z);

		if(dDepth > 0 && dDepth < thickness){
			hit0 = 1;
			break;
		}else{
			search0 = search1;
		}
	}
	float iter_dist = float(i)/float(numLinearIter);
	search1 = search0 + (search1-search0)/2.0;

	int eff_steps = bin_steps*hit0;

	float dist = -1.0;
	for(int i=0;i<eff_steps;++i){
		searchPos = mix(screenStart.xy,screenEnd.xy,search1);
		searchPosUV = searchPos/texSz;

		viewPos = texture(tex_position,searchPosUV);
		viewDistance = abs((rayOrigin.z*rayEnd.z)/mix(rayEnd.z,rayOrigin.z,search1));

		if(dDepth > 0 && dDepth < thickness){
			hit1 = 1;
			dist = search1;
			search1 = search0 + ((search1 - search0) / 2);
		} else {
			float temp = search1;
			search1 = search1 + ((search1 - search0) / 2);
			search0 = temp;
		}
	}

	hit1 = (searchPosUV.x < 0.0 || searchPosUV.x > 1.0||
			searchPosUV.y < 0.0 || searchPosUV.y > 1.0)?
			-1:hit1;
	return vec4(searchPosUV,
				(hit1<0)? -1.0:min(search1,iter_dist),
				dDepth/thickness);
}


void main(){
	vec4 posMask = texture(image_position,TexCoords);
	vec4 normal = vec4(texture(image_normal, TexCoords).xyz, 0.0);

	vec3 color = texture(image_color, TexCoords).rgb;

	if(posMask.w>EPS){
		vec3 reflectionDirection = normalize(reflect(normalize(posMask.xyz), normalize(normal.xyz)));
        if(dot(reflectionDirection,posMask.xyz)>0 && length(posMask.xyz) < LOD1){
            vec4 res = Raycast(posMask.xyz,reflectionDirection,image_position);
			color += (res.z < 0)? vec3(0.0):texture(image_color,res.xy).rgb;
			//FragColor = (res.z < 0)? FragColor:mix(texture(image_color,res.xy)*FragColor/dot(vec3(0.3,0.59,0.11),FragColor),FragColor,clamp(res.z*res.w,0.0,1.0)*(1-posMask.w));
		}
	}
	FragColor = vec4(color,1.0);
}