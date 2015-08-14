#version 330

uniform sampler2D UInputImg;

uniform float camXDim;
uniform float camYDim;
uniform float camXZFactor;
uniform float camYZFactor;

in vec2 pixelPos;
out vec3 LFragment;

void main() {
    float d = float(texture(UInputImg, pixelPos).r);
    LFragment = vec3(1) - vec3((d-500) / 500);
}
