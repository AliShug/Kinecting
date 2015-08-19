#version 330

in vec3 pos;
in vec2 uv;
out vec2 pixelPos;

void main() {
    //vec2 _uv = uv / 2;
    //_uv -= 0.5f;
    //_uv.y = 1-_uv.y;

    // Original grid coord, with w=1
    gl_Position = vec4(pos, 1);
    //gl_Position.x = -gl_Position.x;
    pixelPos = uv;
}
