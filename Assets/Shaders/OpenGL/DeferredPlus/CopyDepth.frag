#version 460 core
#extension GL_ARB_bindless_texture: require
in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D u_ScreenTexture;


void main(){
    gl_FragDepth = texture2D(u_ScreenTexture, TexCoords).r;
}