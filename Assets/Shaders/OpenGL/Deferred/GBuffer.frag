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


struct FragmentOut {   
    vec4 Albedo;
    vec3 Normal;
    vec4 Emissive;
    float Roughness;
    float Metalness;
    float AmbientOclussion;
};

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Tangent;
in vec3 BiTangent;
in vec3 ViewPos;
in mat4 ModelView;


vec3 FromNormalMap(){
    mat3 TBN = mat3(Tangent, BiTangent, Normal);
   
    vec3 normal;
    vec3 normalSample = texture(u_Normal, TexCoord).xyz * 2.0 - 1.0;
    normal = TBN * normalSample;

    return normalize(mat3(ModelView) * normal);
}



void main() {
    FragmentOut result;
    result.Albedo = texture(u_Albedo, TexCoord);
    result.Normal = FromNormalMap();
    result.Roughness = texture(u_MetalRoughness, TexCoord).g;
    result.Metalness = texture(u_MetalRoughness, TexCoord).b;
    result.AmbientOclussion = texture(u_AmbientOcclusion, TexCoord).r;
    result.Emissive = texture(u_Emissive, TexCoord);

    gAlbedo = result.Albedo;
    gNormal = vec4(result.Normal, 1.0);

    gRoughMetalAO.r = result.Roughness;
    gRoughMetalAO.g = result.Metalness;
    gRoughMetalAO.b = result.AmbientOclussion;
    gRoughMetalAO.a = 1.0;


    gPosition.rgb = WorldPos;
    gPosition.a = gl_FragCoord.z;

    gEmissive = result.Emissive;
}