#version 330

uniform sampler2D UInputImg;

in vec2 texCoord;
in vec3 color;
in vec3 normal;

out vec3 fragCol;

void main() {
    vec4 col = texture2D(UInputImg, texCoord);

    fragCol = col.rgb;
    if (col.a < 0.5f) discard;
}
