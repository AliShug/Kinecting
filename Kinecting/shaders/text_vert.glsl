// Accepts screen-space + depth coords, transforms to world-space.
// Mainly for drawing the tracking lines.
#version 330

in vec3 pos;
in vec3 norm;
in vec3 col;
in vec2 uv;

out vec2 texCoord;
out vec3 color;
out vec3 normal;

void main() {
    gl_Position = vec4(pos, 1);
    texCoord = uv;
    color = col;
    normal = norm;
}
