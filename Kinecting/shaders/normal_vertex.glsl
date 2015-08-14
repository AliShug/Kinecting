#version 330

in vec3 LVertexPos3D;
out vec2 pixelPos;

void main() {
    vec2 uv = LVertexPos3D.xy;
    uv /= 2;
    uv -= 0.5f;
    uv.y = 1-uv.y;

    // Original grid coord, with w=1
    gl_Position = vec4(LVertexPos3D, 1);
    //gl_Position.x = -gl_Position.x;
    pixelPos = uv;
}
