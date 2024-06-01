#version 460 core
#extension GL_ARB_bindless_texture: require

#define PI 3.1415926
#define MAX_LIGHTS 1
#define MAX_CASCADE 16

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

uniform sampler2DArray u_ShadowMap;
uniform samplerCube u_IrradianceMap;

layout(bindless_sampler) uniform sampler2D u_GPosition;
layout(bindless_sampler) uniform sampler2D u_GNormal;
layout(bindless_sampler) uniform sampler2D u_GAlbedo;
layout(bindless_sampler) uniform sampler2D u_GEmissive;
layout(bindless_sampler) uniform sampler2D u_GRoughMetalAO;

struct Material {
    vec3 Albedo;
    vec3 Normal;
    float Roughness;
    float Metalness;
    float Ambient;
};

struct Directional{
    vec3 Direction;
    vec3 Color;
};
struct CascadePlane{
    float Plane;
};


struct ShadowMapData{
    CascadePlane CascadePlanes[16];
    int Size;
    float FarPlane;

};

layout (std140, binding = 0) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};

in vec2 TexCoord;

uniform vec3 u_ViewPos;
uniform ShadowMapData u_ShadowMapData;
uniform Directional u_Directional;
uniform mat4 u_View;
uniform mat4 u_Projection;


float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float ShadowCalculationDirectional(vec3 fragPosWorldSpace, vec3 lightPos, vec3 N) {
    // select cascade layer
    vec4 fragPosViewSpace = u_View * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < u_ShadowMapData.Size; ++i)
    {
        if (depthValue < u_ShadowMapData.CascadePlanes[i].Plane)
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = u_ShadowMapData.Size;
    }

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(N);
    float bias = max(0.05 * (1.0 - dot(normal, lightPos)), 0.005);
    const float biasModifier = 0.5f;
    if (layer == int(u_ShadowMapData.Size))
    {
        bias *= 1 / (u_ShadowMapData.FarPlane * biasModifier);
    }
    else
    {
        bias *= 1 / (u_ShadowMapData.CascadePlanes[layer].Plane * biasModifier);
    }



    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(u_ShadowMap, 0));
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_ShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}


vec3 DirectionalRadiance(Directional directional, Material m, vec3 V, vec3 F0, vec3 FragPos){
    vec3 L = normalize(directional.Direction);
    vec3 H = normalize(V + L);

    vec3 radiance = vec3(directional.Color);
    
    float NDF = DistributionGGX(m.Normal, H, m.Roughness);
    float G = GeometrySmith(m.Normal, V, L, m.Roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(m.Normal, V), 0.0) * max(dot(m.Normal, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - m.Metalness;

    float NdotL = max(dot(m.Normal, L), 0.0);
    
    return (kD * m.Albedo / PI + specular) * radiance * NdotL * (1 - ShadowCalculationDirectional(FragPos, directional.Direction, m.Normal));
}

void main(){
    vec3 FragPos = texture(u_GPosition, TexCoord).rgb;
    Material m;
    
    m.Normal = texture(u_GNormal, TexCoord).rgb;
    m.Albedo = pow(texture(u_GAlbedo, TexCoord).rgb, vec3(2.2));
    m.Roughness = texture(u_GRoughMetalAO, TexCoord).r;
    m.Metalness = texture(u_GRoughMetalAO, TexCoord).g;
    m.Ambient = texture(u_GRoughMetalAO, TexCoord).b;

    vec3 V = normalize(u_ViewPos - FragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, m.Albedo, 0);

    vec3 Lo = vec3(0.0);


    Lo += DirectionalRadiance(u_Directional, m, V, F0, FragPos);


    vec3 F = FresnelSchlick(max(dot(m.Normal, V), 0.0), F0);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - m.Metalness;

    vec3 irradiance = texture(u_IrradianceMap, m.Normal).rgb;
    vec3 diffuse = irradiance * m.Albedo;


    vec3 ambient = (kD * diffuse) * m.Ambient;
    vec3 color = Lo + ambient;

    	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
    
    BrightColor = texture(u_GEmissive, TexCoord);
    FragColor = vec4(color, 1.0);
}