#version 330

uniform sampler2D UInputImg;
uniform sampler2D UInputDepth;

uniform float camXDim;
uniform float camYDim;

in vec2 pixelPos;
in float keep;
in float inProjector;
out vec3 LFragment;

void main() {
    // UVs are passed directly from the vertex shader
    vec2 uv  = pixelPos.xy;
    //uv = -uv;

    // Write back color value
    vec3 col = texture(UInputImg, uv).rgb;
    LFragment = (0.5f + 0.5f*inProjector) * col;

    // Write back depth
    //gl_FragDepth = texture(UInputDepth, uv).r / 3000.0f;
    gl_FragDepth = gl_FragCoord.z;

    //if (keep < 0.9999f) discard;
}
