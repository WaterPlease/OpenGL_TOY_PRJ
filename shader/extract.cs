#version 430 core

layout (local_size_x=32, local_size_y=18) in;
layout (rgba16f, binding = 0) readonly uniform image2D image_in;
layout (rgba16f, binding = 1) writeonly uniform image2D image_out;

void main(){
    float d = 0.0;

    ivec2 imgCoord = ivec2(gl_GlobalInvocationID).xy;

    vec4 texel = imageLoad(image_in, imgCoord);

    float brightness = dot(texel.rgb, vec3(0.2126, 0.7152, 0.0722));

    if(brightness < 0.8){
        texel = vec4(0.0, 0.0, 0.0, 1.0);
    }

    imageStore(image_out, imgCoord, texel);
}