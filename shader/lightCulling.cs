# version 430 core

#define FLYDENSITY 256
#define MAX_LIGHTS_TILE 1024
#define NUM_X_AXIS_TILE 24
#define NUM_Y_AXIS_TILE 15
#define NUM_Z_AXIS_TILE 16
#define EPS 0.001

// FLYDENSITY * FLYDENSITY / THREADNUM MUST BE INTEGER
#define THREADNUM 32

layout(local_size_x = THREADNUM, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) readonly buffer flyInfo
{
	float pos [FLYDENSITY*FLYDENSITY*5];
}
flyinfo;

layout(std430, binding = 3) volatile buffer lightIndex
{
	uint indexLstSize;
	uint indexLst[NUM_X_AXIS_TILE*NUM_Y_AXIS_TILE*NUM_Z_AXIS_TILE*MAX_LIGHTS_TILE];
	uvec2 gridCell[NUM_X_AXIS_TILE*NUM_Y_AXIS_TILE*NUM_Z_AXIS_TILE];   // offset, size
}
lightindex;

uniform vec3 xPlanes[NUM_X_AXIS_TILE+1];
uniform vec3 yPlanes[NUM_Y_AXIS_TILE+1];
uniform mat4 view;
uniform float zFar;

shared uint intersectionCnt;
shared uint intersectionLst[MAX_LIGHTS_TILE];

bool InsideOrIntersectPlane(vec3 normal, vec3 center, float radius)
{
	return dot(normal, center) > -radius;
}

bool frustumSphereIntersect(uint lightIdx)
{
	vec3 planes[4];

	planes[0] = xPlanes[gl_WorkGroupID.x];
	planes[1] = -xPlanes[gl_WorkGroupID.x + 1];
	planes[2] = yPlanes[gl_WorkGroupID.y];
	planes[3] = -yPlanes[gl_WorkGroupID.y + 1];

	bool result = true;

	vec3 lightPos = vec3(flyinfo.pos[lightIdx * 5 + 0], flyinfo.pos[lightIdx * 5 + 1], flyinfo.pos[lightIdx * 5 + 2]);
	lightPos = (view * vec4(lightPos, 1.0)).xyz;
	uint zTileID1 = uint(floor(-(lightPos.z-2.0) / zFar * NUM_Z_AXIS_TILE));
	uint zTileID2 = uint(floor(-(lightPos.z+2.0) / zFar * NUM_Z_AXIS_TILE));
	result = result && (gl_WorkGroupID.z == zTileID1 || gl_WorkGroupID.z == zTileID2);
	//result = result && (lightPos.z < (-gl_WorkGroupID.z * zFar / NUM_Z_AXIS_TILE + 2.0));
	//result = result && (lightPos.z > (-(gl_WorkGroupID.z+1) * zFar / NUM_Z_AXIS_TILE - 2.0));
	for (int i = 0; i < 4; i++)
    {
		result = result && InsideOrIntersectPlane(planes[i], lightPos, 2.0);
	}

	return result;
}

void main()
{
	uint lightCnt = FLYDENSITY * FLYDENSITY;
	uint batchCnt = lightCnt / THREADNUM;
	uint tileIndex = gl_WorkGroupID.z * NUM_X_AXIS_TILE * NUM_Y_AXIS_TILE + gl_WorkGroupID.y * NUM_X_AXIS_TILE + gl_WorkGroupID.x;

	//if (gl_WorkGroupID.x + gl_WorkGroupID.y + gl_WorkGroupID.z
	//	== 0)
	//	lightindex.indexLstSize==0;
	if(gl_LocalInvocationIndex==0)
		intersectionCnt = 0;
	barrier();

	for (uint batch=0; batch<batchCnt; batch++)
    {
		uint lightIdx = batch * THREADNUM + gl_LocalInvocationIndex;

        if (frustumSphereIntersect(lightIdx))
        {
			uint idx = atomicAdd(intersectionCnt, 1);
			intersectionLst[idx] = lightIdx;
		}
	}
	barrier();

    if (gl_LocalInvocationIndex == 0)
    {
		uint offset = atomicAdd(lightindex.indexLstSize, intersectionCnt);

		for(int i=0; i<intersectionCnt; i++)
        {
			lightindex.indexLst[offset + i] = intersectionLst[i];
		}
		lightindex.gridCell[tileIndex] = uvec2(offset,intersectionCnt);
	}
}