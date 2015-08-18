#version 330

in vec2 texCoord;
in vec3 color;
in vec3 normal;

out vec3 fragCol;

void main() {
    fragCol = color * (normal+0.5);
}
