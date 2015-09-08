#version 330

uniform sampler2D UInputImg;
uniform sampler2D UPrevDepth;

uniform float camXDim;
uniform float camYDim;
uniform float camXZFactor;
uniform float camYZFactor;

in vec2 pixelPos;
out vec3 LFragment;

#define M_PI 3.1415926535897932384626433832795

void main() {
    vec3 c;
    c.z = float(texture(UInputImg, pixelPos).r);
    float oldZ = float(texture(UPrevDepth, pixelPos).r);
    c.z = (c.z + oldZ) / 2;
    vec2 fac = vec2(camXZFactor, camYZFactor);

    vec2 screenXY = vec2(gl_FragCoord.x / camXDim, gl_FragCoord.y / camYDim);
    screenXY -= 0.5f;
    screenXY *= 2;
    c.xy = screenXY * c.z * fac;
    vec3 norm = normalize(cross(normalize(dFdx(c)), normalize(dFdy(c))));


    LFragment = vec3((norm.xy / 2.0f) + 0.5f, norm.z);
    if (c.z <= 500 || c.z >= 1200) LFragment = vec3(0);

    //if (c.z <= 500.0f || c.z >= 4500.0f) LFragment = vec3(0);
    //LFragment = vec3(dot(gl_FragCoord.xyz, norm));
}
