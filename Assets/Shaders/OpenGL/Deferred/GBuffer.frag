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
layout(bindless_sampler) uniform sampler2D u_HeightMap;


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

in vec3 TangentViewPos;
in vec3 TangentFragPos;

vec3 FromNormalMap(vec2 tex_coords){
    mat3 TBN = mat3(Tangent, BiTangent, Normal);
   
    vec3 normal;
    vec3 normalSample = texture(u_Normal, tex_coords).xyz * 2.0 - 1.0;
    normal = TBN * normalSample;

    return normalize(mat3(ModelView) * normal);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    float height =  texture(u_HeightMap, texCoords).r;    
    vec2 p = viewDir.xy / viewDir.z * (height * 0.1);
    return texCoords - p;
}

void main() {
    vec3 view_dir = normalize(TangentViewPos - TangentFragPos);
    vec2 tex_coords = ParallaxMapping(TexCoord, view_dir);

    FragmentOut result;
    result.Albedo = texture(u_Albedo, tex_coords);
    result.Normal = FromNormalMap(tex_coords);
    result.Roughness = texture(u_MetalRoughness, tex_coords).g;
    result.Metalness = texture(u_MetalRoughness, tex_coords).b;
    result.AmbientOclussion = texture(u_AmbientOcclusion, tex_coords).r;
    result.Emissive = texture(u_Emissive, tex_coords);

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