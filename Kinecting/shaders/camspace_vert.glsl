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

uniform mat4 MatMVP;
uniform float camXZFactor;
uniform float camYZFactor;

void main() {
    // Calculate camera-space position of vertex
    vec3 newPos = pos;
    vec2 factor = vec2(camXZFactor, camYZFactor);
    newPos.xy *= newPos.z * factor;

    gl_Position = MatMVP * vec4(newPos, 1);
    texCoord = uv;
    color = col;
    normal = norm;
}
