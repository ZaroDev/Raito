#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat3 u_NormalMatrix;

out vec3 WorldPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
    TexCoord = aTexCoord;

    WorldPos = vec3(u_Model * vec4(aPos, 1.0));
    Normal = u_NormalMatrix * aNormal;

    gl_Position = u_Projection * u_View * vec4(WorldPos, 1.0);
}