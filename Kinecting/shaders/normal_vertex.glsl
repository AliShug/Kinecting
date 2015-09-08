#version 330

in vec3 pos;
in vec2 uv;

out vec2 pixelPos;
out float inProjector;
out float keep;

uniform sampler2D UInputDepth;

uniform mat4 MatMVP;
uniform mat4 MatMVP_projector;
uniform float kinectXZFactor;
uniform float kinectYZFactor;

void main() {
    //vec2 _uv = uv / 2;
    //_uv -= 0.5f;
    //_uv.y = 1-_uv.y;

    // Calculate camera-space position of vertex (fragment)
    vec3 newPos = pos;
    vec2 factor = vec2(kinectXZFactor, kinectYZFactor);
    newPos.z = texture(UInputDepth, uv).r / 1000.0f;
    newPos.xy *= newPos.z * factor;

    gl_Position = MatMVP * vec4(newPos, 1);
    vec4 projectorPos = MatMVP_projector * vec4(newPos, 1);
    projectorPos /= projectorPos.w;
    inProjector = float(projectorPos.x > -1 && projectorPos.x < 1 &&
        projectorPos.y > 0 && projectorPos.y < 1);

    pixelPos = uv;

    keep = 1.0f;
    if (newPos.z < 0.5f || newPos.z > 4.5f) keep = 0.0f;
}
