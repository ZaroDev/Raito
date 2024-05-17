#version 460 core
#extension GL_ARB_bindless_texture: require

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;

layout(bindless_sampler) uniform sampler2D u_Albedo;


in vec3 Normal;
in vec2 TexCoord;
in vec3 WorldPos;

void main() {
    gPosition = vec4(WorldPos, 1.0);
    gNormal = vec4(normalize(Normal), 1.0);
    gAlbedo.rgb = texture(u_Albedo, TexCoord).rgb;
    gAlbedo.a = 1;
}