#version 460 core
#extension GL_ARB_bindless_texture: require

layout(location = 0) out vec4 Position;
layout(location = 1) out vec4 Normals;
layout(location = 2) out vec4 FragColor;

in vec3 Normal;
in vec3 WorldPos;

void main() {
    Position = vec4(WorldPos, 1.0);
    Normals = vec4(Normal, 1.0);
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}