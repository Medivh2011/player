#version 300 es
precision mediump float;
in vec4 av_Position;
in vec2 af_Position;
out vec2 v_texPosition;
void main() {
    v_texPosition = af_Position;
    gl_Position = av_Position;
}
