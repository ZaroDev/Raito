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



in vec4 WorldPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Tangent;
in vec3 BiTangent;
in vec3 ViewPos;
in mat4 ModelView;

in vec3 TangentViewPos;
in vec3 TangentFragPos;

vec3 fromNormalMap(vec2 tex_coords, float lod){
    mat3 TBN = mat3(Tangent, BiTangent, Normal);
   
    vec3 normal;
    vec3 normalSample = textureLod(u_Normal, tex_coords, lod).rgb * 2.0 - 1.0;
    normal = TBN * normalSample;

    return normalize(mat3(ModelView) * normal);
}


void main() {
   vec2 ddxTexCoord = dFdx(TexCoord);
   vec2 ddyTexCoord = dFdy(TexCoord);
   float ddxLength = length(ddxTexCoord);
   float ddyLength = length(ddyTexCoord);
   float derivativeLength = max(ddxLength, ddyLength);
   float lod = log2(derivativeLength);

   gPosition = WorldPos;
   gNormal = vec4(fromNormalMap(TexCoord, lod), 1.0);
   gAlbedo = textureLod(u_Albedo, TexCoord, lod);
   gEmissive = textureLod(u_Emissive, TexCoord, lod);
   gRoughMetalAO.rg = textureLod(u_MetalRoughness, TexCoord, lod).gb;
   gRoughMetalAO.b = textureLod(u_AmbientOcclusion, TexCoord, lod).r;
   gRoughMetalAO.a = 1.0;
}