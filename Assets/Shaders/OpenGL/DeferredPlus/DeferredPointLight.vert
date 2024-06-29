#version 460 core

layout(location = 0) in vec3 aPos;


layout(std430, binding = 0) readonly buffer Models {
    mat4 Model[];
};
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out mat4 InverseProjView;
out flat uint LightID;
void main() {
    LightID = gl_InstanceID;
    gl_Position = u_Projection * u_View * Model[LightID] * vec4(aPos, 1.0);
    InverseProjView = inverse(u_Projection * u_View);
}