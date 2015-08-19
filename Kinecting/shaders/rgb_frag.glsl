#version 330

uniform sampler2D UInputImg;
uniform sampler2D UInputDepth;

uniform float camXDim;
uniform float camYDim;

in vec2 pixelPos;
out vec3 LFragment;

void main() {
    // UVs are passed directly from the vertex shader
    vec2 uv  = pixelPos.xy;
    //uv = -uv;

    // Write back color value
    vec3 col = texture(UInputImg, uv).rgb;
    LFragment = col;//vec3(0, 0, col.b);

    // Write back depth
    gl_FragDepth = texture(UInputDepth, uv).r / 3000.0f;

    // Hacky target icon thing
    vec2 pos = gl_FragCoord.xy;
    pos.x /= camXDim;
    pos.y /= camYDim;

    float dist = distance(pos, vec2(0.5f, 0.5f));
    if (dist > 0.01f && dist < 0.013f) {
        LFragment = vec3(1);
        gl_FragDepth = 0.0f;
    }
}
