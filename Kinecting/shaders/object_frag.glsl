#version 330

in vec2 texCoord;
in vec3 color;

out vec3 fragCol;

void main() {
    fragCol = color;
}
