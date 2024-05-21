#version 460 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 Normal;
in vec2 TexCoord;
in vec3 WorldPos;

uniform vec3 u_Color;

void main() 
{
    FragColor = vec4(u_Color, 1.0);
    BrightColor = vec4(u_Color, 1.0);
}