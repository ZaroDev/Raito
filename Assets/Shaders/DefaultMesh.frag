#version 460 core
#extension GL_ARB_bindless_texture: require

layout(bindless_sampler) uniform sampler2D u_Albedo;
layout(location = 0) out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoord;
in vec3 WorldPos;

void main() {
    FragColor = texture(u_Albedo, TexCoord);
}