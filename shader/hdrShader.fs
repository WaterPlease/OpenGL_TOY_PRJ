#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float gamma;

void main()
{
    vec3 hdr = texture(screenTexture, TexCoords).rgb;

    const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;
    //vec3 color = clamp((hdr * (A * hdr + B)) / (hdr * (C * hdr + D) + E),0.0,1.0);

    //vec3 mapped = color/(color+vec3(1.0));

    vec3 mapped = pow(hdr,vec3(1.0/gamma));
    FragColor = vec4(mapped,1.0);
}