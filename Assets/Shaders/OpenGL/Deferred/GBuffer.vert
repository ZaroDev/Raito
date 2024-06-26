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


out vec4 WorldPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 Tangent;
out vec3 BiTangent;
out vec3 ViewPos;
out mat4 ModelView;

out vec3 TangentViewPos;
out vec3 TangentFragPos;

void main() {
    TexCoord = aTexCoord;

    WorldPos = u_Model * vec4(aPos, 1.0);
    Normal = u_NormalMatrix * aNormal;

    Tangent = aTangent;
    BiTangent = aBiTangent;

    ViewPos = vec3(inverse(u_View)[3]);

    ModelView = u_View * u_Model;

    vec3 T   = normalize(mat3(u_Model) * aTangent);
    vec3 B   = normalize(mat3(u_Model) * aBiTangent);
    vec3 N   = normalize(mat3(u_Model) * aNormal);
    mat3 TBN = transpose(mat3(T, B, N));

    TangentViewPos  = TBN * ViewPos;
    TangentFragPos  = TBN * WorldPos;

    gl_Position = u_Projection * u_View * vec4(WorldPos, 1.0);
}