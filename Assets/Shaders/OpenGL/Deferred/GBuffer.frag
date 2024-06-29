#version 460 core
#extension GL_ARB_bindless_texture: require

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;
layout(location = 3) out vec4 gEmissive;
layout(location = 4) out vec4 gRoughMetalAO;

layout(bindless_sampler) uniform sampler2D u_Albedo;
layout(bindless_sampler) uniform sampler2D u_Normal;
layout(bindless_sampler) uniform sampler2D u_Emissive;
layout(bindless_sampler) uniform sampler2D u_MetalRoughness;
layout(bindless_sampler) uniform sampler2D u_AmbientOcclusion;



in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;
in mat4 ModelView;

vec3 fromNormalMap(){
    vec3 tangentNormal = texture(u_Normal, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}


void main() {
   gPosition = vec4(WorldPos, 1.0);
   gNormal = vec4(fromNormalMap(), 1.0);
   gAlbedo = texture(u_Albedo, TexCoords);
   gEmissive = texture(u_Emissive, TexCoords);
   gRoughMetalAO.rg = texture(u_MetalRoughness, TexCoords).gb;
   gRoughMetalAO.b = texture(u_AmbientOcclusion, TexCoords).r;
   gRoughMetalAO.a = 1.0;
}