#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBiTangent;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat3 u_NormalMatrix;


out vec3 WorldPos;
out vec3 Normal;
out vec2 TexCoords;
out mat4 ModelView;

void main() {
    TexCoords = aTexCoord;

    WorldPos = vec3(u_Model * vec4(aPos, 1.0));
    Normal = u_NormalMatrix * aNormal;

    ModelView = u_View * u_Model;
    gl_Position = u_Projection * u_View * vec4(WorldPos.xyz, 1.0);
}