#version 330

uniform sampler2D UInputImg;
uniform sampler2D UPrevDepth;

uniform float camXDim;
uniform float camYDim;
uniform float camXZFactor;
uniform float camYZFactor;

in vec2 pixelPos;
out vec3 LFragment;

#define M_PI 3.1415926535897932384626433832795

/*vec3 triNormal(vec3 p1, vec3 p2, vec3 p3) {
    vec3 u = p2 - p1;
    vec3 v = p3 - p1;
    return cross(u, v);
}

vec3 normal(vec2 uv) {
    vec3 l, r, u, d, c;
    l.xy = uv - vec2(1.0f / camXDim, 0.00f);
    r.xy = uv + vec2(1.0f / camXDim, 0.00f);
    u.xy = uv - vec2(0.00f, 1.0f / camYDim);
    d.xy = uv + vec2(0.00f, 1.0f / camYDim);
    c.xy = uv;

    l.z = float(texture(UInputImg, l.xy).r) / 9.8f;
    r.z = float(texture(UInputImg, r.xy).r) / 9.8f;
    u.z = float(texture(UInputImg, u.xy).r) / 9.8f;
    d.z = float(texture(UInputImg, d.xy).r) / 9.8f;
    c.z = float(texture(UInputImg, c.xy).r) / 9.8f;

    // Bring into camera space
    vec2 fac = vec2(camXZFactor, camYZFactor) / 1000.0f;
    r.xy *= r.z * fac;
    u.xy *= u.z * fac;
    d.xy *= d.z * fac;
    c.xy *= c.z * fac;

    vec3 n1, n2;
    return triNormal(c, l, u) * 10;
}*/

void main() {
    /*
    // UVs are passed directly from the vertex shader
    vec2 uv  = pixelPos.xy;
    vec2 uvL = uv - vec2(1.0f / camXDim, 0.00f);
    vec2 uvR = uv + vec2(1.0f / camXDim, 0.00f);
    vec2 uvU = uv - vec2(0.00f, 1.0f / camYDim);
    vec2 uvD = uv + vec2(0.00f, 1.0f / camYDim);

    // Get the surrounding depth values
    float depthL = float(texture(UInputImg, uvL).r);
    float depthR = float(texture(UInputImg, uvR).r);
    float depthU = float(texture(UInputImg, uvU).r);
    float depthD = float(texture(UInputImg, uvD).r);

    // Calculate the normal
    float dx = depthR - depthL;
    float dy = depthD - depthU;
    dx /= 50.0f;
    dy /= 50.0f;

    dx = clamp(dx, -1, 1);
    dy = clamp(dy, -1, 1);
    vec3 testNorm = vec3(dx, dy, sqrt(1 - dx*dx - dy*dy));
    testNorm.xy /= 2;
    testNorm.xy += 0.5;

    // Write back
    LFragment = testNorm;*/

    //vec3 n = normal(pixelPos);
    //LFragment = n.brg;

    vec3 c;
    c.z = float(texture(UInputImg, pixelPos).r);
    float oldZ = float(texture(UPrevDepth, pixelPos).r);
    c.z = (c.z + oldZ) / 2;
    vec2 fac = vec2(camXZFactor, camYZFactor);

    vec2 screenXY = vec2(gl_FragCoord.x / camXDim, gl_FragCoord.y / camYDim);
    screenXY -= 0.5f;
    screenXY *= 2;
    c.xy = screenXY * c.z * fac;
    //c /= 10000.0f;
    vec3 norm = normalize(cross(normalize(dFdx(c)), normalize(dFdy(c))));

    //float angle = dot(norm, vec3(0, 0, 1));
    //LFragment = vec3(0, 0, angle);

    LFragment = vec3((norm.xy / 2.0f) + 0.5f, norm.z);
    //LFragment = vec3(norm.x / 2.0f, 0, 0);
    if (c.z <= 500 || c.z >= 1200) LFragment = vec3(0);

    //if (c.z <= 500.0f || c.z >= 4500.0f) LFragment = vec3(0);
    //LFragment = vec3(dot(gl_FragCoord.xyz, norm));
}
