#version 330

in vec3 pos;
in vec2 uv;

out vec2 pixelPos;
out float keep;

uniform sampler2D UInputDepth;

uniform mat4 MatMVP;
uniform float camXZFactor;
uniform float camYZFactor;

void main() {
    //vec2 _uv = uv / 2;
    //_uv -= 0.5f;
    //_uv.y = 1-_uv.y;

    // Calculate camera-space position of vertex (fragment)
    vec3 newPos = pos;
    vec2 factor = vec2(camXZFactor, camYZFactor);
    newPos.z = texture(UInputDepth, uv).r / 1000.0f;
    newPos.xy *= newPos.z * factor;

    gl_Position = MatMVP * vec4(newPos, 1);
    //gl_Position.x = -gl_Position.x;
    pixelPos = uv;

    keep = 1.0f;
    if (newPos.z < 0.5f || newPos.z > 4.5f) keep = 0.0f;
}
