#version 460 core
#extension GL_ARB_bindless_texture: require

#define PI 3.1415926
#define GAMMA 2.2


layout(location = 2) out vec4 FragColor;

layout(location = 0) uniform samplerCube u_IrradianceMap;
layout(location = 1) uniform samplerCube u_PrefilterMap;

layout(bindless_sampler) uniform sampler2D u_GPosition;
layout(bindless_sampler) uniform sampler2D u_GNormal;
layout(bindless_sampler) uniform sampler2D u_GAlbedo;
layout(bindless_sampler) uniform sampler2D u_GEmissive;
layout(bindless_sampler) uniform sampler2D u_GRoughMetalAO;
layout(bindless_sampler) uniform sampler2D u_SSAO;
layout(bindless_sampler) uniform sampler2D u_BRDFLUT;

in vec2 TexCoords;

uniform vec3 u_ViewPosition;

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

void main(){
    vec3 fragPos = texture(u_GPosition, TexCoords).rgb;
    vec3 N = texture(u_GNormal, TexCoords).rgb;
    vec3 albedo = pow(texture(u_GAlbedo, TexCoords).rgb, vec3(GAMMA));
    float roughness = texture(u_GRoughMetalAO, TexCoords).r;
    float metallic = texture(u_GRoughMetalAO, TexCoords).g;
    float occlusion = texture(u_SSAO, TexCoords).b;

    vec3 V = normalize(u_ViewPosition - fragPos);
    vec3 R = reflect(-V, N); 
   

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(u_BRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * occlusion;
    
    FragColor = vec4(ambient, 1.0);
}