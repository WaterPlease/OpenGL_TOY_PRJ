#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomTexture;
uniform float gamma;

vec3 ACESFilm(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e),0.0,1.0);
}
float exposure = 1.0f;
void main()
{
    vec3 hdr = texture(screenTexture, TexCoords).rgb;

    const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;

    hdr += texture(bloomTexture,TexCoords).rgb;

    vec3 mapped = vec3(1.0) - exp(-hdr * exposure);//ACESFilm(hdr);

    mapped = pow(mapped,vec3(1.0/gamma));
    FragColor = vec4(mapped,1.0);
}