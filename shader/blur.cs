#version 430 core

layout (local_size_x=1, local_size_y=1) in;
layout (binding = 0) uniform sampler2D depth_in;
layout (r32f, binding = 1) writeonly uniform image2D depth_out;

void main(){
    float d = 0.0;
    ivec2 minCoords = ivec2(0);
    ivec2 maxCoords = ivec2(imageSize(depth_in)-1);

    ivec2 imgCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 tmpCoord;
    for(int i = -2; i < 3; i++)
    {
        for (int j = -2; j < 3; j++)
        {
            tmpCoord = max(min(imgCoord + ivec2(i, j), maxCoords), minCoords);
            d += imageLoad(depth_in, tmpCoord);
        }
    }
    d /= 25.0;

    imageStore(depth_out,imgCoord,d);
}