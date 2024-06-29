#version 460 core
#extension GL_ARB_bindless_texture: require

#define PI 3.1415926
#define GAMMA 2.2

layout(location = 1) out vec4 FragColor;


layout(bindless_sampler) uniform sampler2D u_GPosition;
layout(bindless_sampler) uniform sampler2D u_GNormal;
layout(bindless_sampler) uniform sampler2D u_GAlbedo;
layout(bindless_sampler) uniform sampler2D u_GRoughMetalAO;

uniform vec2 u_ScreenSize;
uniform vec3 u_ViewPosition;

in flat uint LightID;

layout(std430, binding = 1) readonly buffer PointBuffer{
    mat3 point[2048];
};

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 directionalLightRadiance(vec3 direction, vec3 color,vec3 viewDir, vec3 F0, vec3 normal, float roughness, float metallic, vec3 albedo){
    vec3 fragToLightDir = normalize(-direction);
    vec3 halfwayDir = normalize(fragToLightDir + viewDir);
    vec3 radiance = color;

    vec3 fresnel = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
    float normalDistributionFunction = distributionGGX(normal, halfwayDir, roughness);
    float geometry = geometrySmith(normal, viewDir, fragToLightDir, roughness);

    vec3 kSpecular = fresnel;
    vec3 kDiffuse = vec3(1.0) - kSpecular;
    kDiffuse *= 1.0 - metallic;

    vec3 numerator = normalDistributionFunction * geometry * fresnel;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, fragToLightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    float normalDotFragToLightDir = max(dot(normal, fragToLightDir), 0.0);
    return (kDiffuse * albedo / PI + specular) * radiance * normalDotFragToLightDir;
}


vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

void main(){
    vec2 texCoords = gl_FragCoord.xy / u_ScreenSize;

    vec3 fragPos = texture(u_GPosition, texCoords).rgb;
    vec3 N = texture(u_GNormal, texCoords).rgb;
    vec3 albedo = pow(texture(u_GAlbedo, texCoords).rgb, vec3(GAMMA));
    float roughness = texture(u_GRoughMetalAO, texCoords).r;
    float metallic = texture(u_GRoughMetalAO, texCoords).g;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 V = normalize(u_ViewPosition - fragPos);
    
    vec3 lighPos = point[LightID][0];
    vec3 lightColor = point[LightID][1];
    // calculate per-light radiance
    vec3 L = normalize(lighPos - fragPos);
    vec3 H = normalize(V + L);
    float distance = length(lighPos - fragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lightColor * attenuation;

    // Cook-Torrance BRDF
    float NDF = distributionGGX(N, H, roughness);   
    float G   = geometrySmith(N, V, L, roughness);    
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
        
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	                
            
    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);        
    FragColor = vec4((kD * albedo / PI + specular) * radiance * NdotL, 1.0);
}