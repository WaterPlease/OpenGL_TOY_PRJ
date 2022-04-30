# version 430 core

#define FLYDENSITY 32

// FLYDENSITY * FLYDENSITY / THREADNUM MUST BE INTEGER
#define THREADNUM 8

layout(local_size_x = THREADNUM, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) readonly buffer flyInfo
{
	float pos [FLYDENSITY*FLYDENSITY*5];
}
flyinfo;

layout(std430, binding = 3) volatile buffer lightIndex
{
	uint indexLstSize;
	uint indexLst[48*30*1024];
	uvec2 gridCell[48*30];   // offset, size
}
lightindex;

uniform vec3 xPlanes[49];
uniform vec3 yPlanes[31];
uniform mat4 view;

shared uint intersectionCnt;
shared uint intersectionLst[1024];

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

	int s=0;
	bool result = true;
	for(int i = 0; i < 4; i++)
    {
		vec3 lightPos = vec3(flyinfo.pos[lightIdx * 5 + 0], flyinfo.pos[lightIdx * 5 + 1], flyinfo.pos[lightIdx * 5 + 2]);
		lightPos = (view * vec4(lightPos, 1.0)).xyz;
		result = result && InsideOrIntersectPlane(planes[i], lightPos, 2.0);
	}

	return result;
}

void main()
{
	uint lightCnt = FLYDENSITY * FLYDENSITY;
	uint batchCnt = lightCnt / THREADNUM;
	uint tileIndex = gl_WorkGroupID.z * 48*30 + gl_WorkGroupID.y * 48 + gl_WorkGroupID.x;

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