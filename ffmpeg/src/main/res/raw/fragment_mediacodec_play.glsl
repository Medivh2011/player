#version 300 es
//OpenGL ES3.0外部纹理扩展
#extension GL_OES_EGL_image_external_essl3 : require

precision mediump float;
in vec2 v_texPosition;
uniform samplerExternalOES sTexture;
out vec4 fragColor;

void main() {
    fragColor=texture(sTexture, v_texPosition);
}
