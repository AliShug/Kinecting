#version 330

uniform mat4 MatMVP;

in vec3 pos;
in vec3 norm;
in vec3 col;
in vec2 uv;

out vec2 texCoord;
out vec3 color;

void main() {
    // TODO
    gl_Position = MatMVP * vec4(pos, 1);
    texCoord = uv;
    color = col;
}
