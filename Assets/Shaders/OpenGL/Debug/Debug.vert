#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 Normal;

void main() {
    Normal = aNormal;
    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
}