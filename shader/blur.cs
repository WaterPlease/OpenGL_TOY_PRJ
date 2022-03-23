#version 430 core

layout(local_size_x = 32, local_size_y = 18) in;
layout(rgba16f, binding = 0) readonly uniform image2D image_in;
layout(rgba16f, binding = 1) writeonly uniform image2D image_out;

float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main(){
    float d = 0.0;

    ivec2 imgCoord = ivec2(gl_GlobalInvocationID).xy;

    vec4 texel = imageLoad(image_in, imgCoord) * weight[0];

    ivec2 offset = ivec2(1,0);

    for(int d = 0; d < 2; d++){
        for(int i = 1; i < 5; i++){
            texel += imageLoad(image_in, imgCoord + offset * i) * weight[i];
            texel += imageLoad(image_in, imgCoord - offset * i) * weight[i];
        }
        offset = ivec2(1, 1) - offset;
    }

    imageStore(image_out, imgCoord, vec4(texel.rgb,1.0));
}