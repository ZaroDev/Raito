#version 460 core

layout(location = 0) in vec3 aPos;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out mat4 InverseProjView;

void main() {

    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
    InverseProjView = inverse(u_Projection * u_View);
}