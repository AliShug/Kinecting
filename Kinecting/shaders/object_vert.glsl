// Accepts conventional-style object meshes and transforms to world space.
#version 330

in vec3 pos;
in vec3 norm;
in vec3 col;
in vec2 uv;

out vec2 texCoord;
out vec3 color;
out vec3 normal;

uniform mat4 MatMVP;

void main() {
    gl_Position = MatMVP * vec4(pos, 1);
    texCoord = uv;
    color = col;
    normal = norm;
}
