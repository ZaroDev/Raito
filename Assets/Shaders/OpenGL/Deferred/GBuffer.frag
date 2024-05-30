#version 460 core
#extension GL_ARB_bindless_texture: require

layout(location = 0) out vec4 AlbedoOut;
layout(location = 1) out vec4 NormalOut;
layout(location = 2) out vec4 RoughnessOut;
layout(location = 3) out vec4 MetalnessOut;

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


in vec3 Normal;
in vec2 TexCoord;
in mat4 ModelView;
in vec3 Tangent;
in vec3 BiTangent;

vec4 AlbedoBase = vec4(1.0f);
float RoughnessBase = 1.0f;
float MetalnessBase = 0.0f;
vec4 EmissiveBase = vec4(0.0);

void TexturedMesh(out FragmentOut result){

    mat3 tbn;
    tbn[0] = Tangent;
    tbn[1] = BiTangent;
    tbn[2] = Normal;


    vec3 normal;
    vec3 normalSample = texture(u_Normal, TexCoord).xyz * 2.0 - 1.0;
    normal = tbn * normalSample;

    result.Normal = normalize(mat3(ModelView) * normal);
    
    result.Albedo = AlbedoBase;
    result.Albedo *= texture(u_Albedo, TexCoord);

    result.Roughness = RoughnessBase;
    result.Roughness *= texture(u_MetalRoughness, TexCoord).g;
    
    result.Metalness = MetalnessBase;
    result.Metalness += texture(u_MetalRoughness, TexCoord).b;

    result.Emissive = EmissiveBase;
    result.Emissive += texture(u_Emissive, TexCoord);

    result.AmbientOclussion = 1.0f;
    result.AmbientOclussion *= texture(u_AmbientOcclusion, TexCoord).r;
}

void main() {
    FragmentOut fragmentOut;
    TexturedMesh(fragmentOut);
    AlbedoOut = fragmentOut.Albedo;
    NormalOut = vec4(fragmentOut.Normal, 1.0);
    RoughnessOut =  fragmentOut.Emissive;
    MetalnessOut = vec4(0.0, fragmentOut.Roughness, fragmentOut.Metalness, 1.0);
}