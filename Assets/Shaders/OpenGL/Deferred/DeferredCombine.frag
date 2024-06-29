#version 460 core
#extension GL_ARB_bindless_texture: require

#define PI 3.1415926
#define GAMMA 2.2

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

uniform sampler2DArray u_ShadowMap;
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;


layout(bindless_sampler) uniform sampler2D u_GPosition;
layout(bindless_sampler) uniform sampler2D u_GNormal;
layout(bindless_sampler) uniform sampler2D u_GAlbedo;
layout(bindless_sampler) uniform sampler2D u_GEmissive;
layout(bindless_sampler) uniform sampler2D u_GRoughMetalAO;
layout(bindless_sampler) uniform sampler2D u_SSAO;
layout(bindless_sampler) uniform sampler2D u_BRDFLUT;


layout(std430, binding = 0) readonly buffer ShadowMapData{
    int shadowMapSize;
    float farPlane;
    float cascadePlanes[16];
};
layout(std140, binding = 1) readonly buffer LigthSpaceMatrices{
    mat4 lightSpaceMatrices[16];
};

layout(std430, binding = 2) readonly buffer DirectionalBuffer{
    mat3 directional;
};
layout(std430, binding = 3) readonly buffer PointBuffer{
    mat3 point[2048];
};
uniform int u_PointSize;
uniform mat4 u_View;
uniform vec3 u_ViewPosition;
in vec2 TexCoord;

float shadowCalculationDirectional(vec3 fragPosWorldSpace, vec3 lightDir, vec3 N) {
    // select cascade layer
    vec4 fragPosViewSpace = u_View * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < shadowMapSize; ++i) {
        if (depthValue < cascadePlanes[i]) {
            layer = i;
            break;
        }
    }
    if (layer == -1) {
        layer = shadowMapSize;
    }

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0) {
        return 0.0;
    }
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(N);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    const float biasModifier = 0.5f;
    if (layer == int(shadowMapSize)) {
        bias *= 1 / (farPlane * biasModifier);
    }
    else {
        bias *= 1 / (cascadePlanes[layer] * biasModifier);
    }
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(u_ShadowMap, 0));
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(u_ShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

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
    vec3 fragPos = texture(u_GPosition, TexCoord).rgb;
    vec3 N = texture(u_GNormal, TexCoord).rgb;
    vec3 albedo = pow(texture(u_GAlbedo, TexCoord).rgb, vec3(GAMMA));
    float roughness = texture(u_GRoughMetalAO, TexCoord).r;
    float metallic = texture(u_GRoughMetalAO, TexCoord).g;
    float occlusion = texture(u_SSAO, TexCoord).b;

    vec3 V = normalize(u_ViewPosition - fragPos);
    vec3 R = reflect(-V, N); 
    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    {
        vec3 directionalDir = directional[0];
        vec3 directionalColor = directional[1];
        float vis = shadowCalculationDirectional(fragPos, directionalDir, N);


        // calculate per-light radiance
        vec3 L = normalize(directionalDir);
        vec3 H = normalize(V + L);
        vec3 radiance = directionalColor;

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

        // add to outgoing radiance Lo
        Lo += ((kD * albedo / PI + specular) * radiance * NdotL) * (1 - vis);
    }


    for(int i = 0; i < u_PointSize; i++){
        vec3 lighPos = point[i][0];
        vec3 lightColor = point[i][1];
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

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

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

    vec3 color = (ambient * 0.1) + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/GAMMA)); 

    FragColor = vec4(color, 1.0);
    BrightColor = pow(texture(u_GEmissive, TexCoord), vec4(GAMMA));
}