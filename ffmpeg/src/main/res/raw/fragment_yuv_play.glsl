#version 300 es
precision mediump float;
in vec2 v_texPosition;
uniform sampler2D sampler_y;
uniform sampler2D sampler_u;
uniform sampler2D sampler_v;
out vec4 fragColor;
void main() {
    float y,u,v;
    y = texture(sampler_y,v_texPosition).r;
    u = texture(sampler_u,v_texPosition).r- 0.5;
    v = texture(sampler_v,v_texPosition).r- 0.5;

    vec3 rgb;
    rgb.r = y + 1.403 * v;
    rgb.g = y - 0.344 * u - 0.714 * v;
    rgb.b = y + 1.770 * u;

    fragColor = vec4(rgb,1);
}
