#version 460 core
#extension GL_ARB_bindless_texture: require

#define PI 3.1415926
#define GAMMA 2.2

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

uniform sampler2DArray u_ShadowMap;
uniform samplerCube u_IrradianceMap;
//uniform samplerCube u_PrefilterMap;


layout(bindless_sampler) uniform sampler2D u_Albedo;
layout(bindless_sampler) uniform sampler2D u_Emissive;
layout(bindless_sampler) uniform sampler2D u_RoughMetalAO;
layout(bindless_sampler) uniform sampler2D u_Normal;
layout(bindless_sampler) uniform sampler2D u_HeightMap;
layout(bindless_sampler) uniform sampler2D u_SSAO;
layout(bindless_sampler) uniform sampler2D u_BRDFLUT;

struct Material {
    vec3 Albedo;
    vec3 Normal;
    float Roughness;
    float Metallic;
    float Ambient;
};

struct Directional{
    vec3 Direction;
    vec3 Position;
    vec3 Color;
};

struct Point{
    vec3 Position;
    vec3 Color;
};

layout(std430, binding = 0) readonly buffer ShadowMapData{
    int shadowMapSize;
    float farPlane;
    float cascadePlanes[16];
};
layout(std140, binding = 1) readonly buffer LigthSpaceMatrices{
    mat4 lightSpaceMatrices[16];
};

layout(std430, binding = 2) readonly buffer DirectionalBuffer{
    Directional directional;
};
layout(std430, binding = 3) readonly buffer PointBuffer{
    uint pointSize;
    Point point[1024];
};

uniform mat4 u_InvView;
uniform mat4 u_View;
uniform vec3 u_ViewPosition;
uniform int u_EnableParallax;

in vec3 WorldPos;
in vec2 TexCoord;
in vec3 Normal;
in vec3 Tangent;
in vec3 BiTangent;
in mat4 ModelView;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

float shadowCalculationDirectional(vec3 fragPosWorldSpace, vec3 lightPos, vec3 N) {
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
    float bias = max(0.05 * (1.0 - dot(normal, lightPos)), 0.005);
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
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = geometrySchlickGGX(NdotV, roughness);
    float ggx1  = geometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}
vec3 cookTorrenceSpecularBRDF(vec3 F, vec3 N, vec3 V, vec3 H, vec3 L, float roughness) {
  float D = distributionGGX(N, H, roughness);
  float G = geometrySmith(N, V, L, roughness);

  vec3 numerator    = D * G * F;
  float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
  vec3 specular     = numerator / max(denominator, 0.001);

  return specular;
}

vec3 directionalLightRadiance(Directional directional, Material m, vec3 F0, vec3 worldPos, vec3 V){
    vec3 L = normalize(directional.Direction);
    vec3 H = normalize(L + V);
    // cos(angle) between surface normal and light
    float NdL = max(0.001, dot(m.Normal, L));

    // cos(angle) between surface half vector and eye
    float HdV = max(0.001, dot(H, V));
    
    vec3 F = fresnelSchlick(HdV, F0);
    vec3 kD =  vec3(1.0) - F;

    vec3 specBrdf = cookTorrenceSpecularBRDF(F, m.Normal, V, H, L, m.Roughness);
    vec3 diffuseBrdf = kD * (m.Albedo / PI) * (1.0 - m.Metallic); // Lambert diffuse

    float A = mix(1.0f, 1.0 / (1.0 + 0.1 * dot(directional.Position - worldPos, directional.Position - worldPos)), 0);

    vec3 radiance = A * vec3(1.0);
    return (specBrdf + diffuseBrdf) * radiance * NdL;
}

vec3 pointLightRadiance(Point point, Material m, vec3 F0, vec3 worldPos, vec3 V){
    vec3 L = normalize(point.Position - worldPos);
    vec3 H = normalize(V + L);

    // cos(angle) between surface normal and light
    float NdL = max(0.001, dot(m.Normal, L));

    // cos(angle) between surface half vector and eye
    float HdV = max(0.001, dot(H, V));
    
    vec3 F = fresnelSchlick(HdV, F0);
    vec3 kD =  vec3(1.0) - F;

    vec3 specBrdf = cookTorrenceSpecularBRDF(F, m.Normal, V, H, L, m.Roughness);
    vec3 diffuseBrdf = kD * (m.Albedo / PI) * (1.0 - m.Metallic); // Lambert diffuse

    float A = mix(1.0f, 1.0 / (1.0 + 0.1 * dot(point.Position - worldPos, point.Position - worldPos)), 1);

    vec3 radiance = A * directional.Color;
    return (specBrdf + diffuseBrdf) * radiance * NdL;
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

/*vec3 IBLAmbientRadiance(vec3 N, vec3 V, Material m, vec3 F0){
    vec3 worldN = (u_InvView * vec4(N, 0)).xyz; //World normal
    vec3 worldV = (u_InvView * vec4(V, 0)).xyz; //World view
    vec3 irradiance = texture(u_IrradianceMap, worldN).xyz;

    float NdV = max(0.001, dot(worldN, worldV));
    vec3 kS = fresnelSchlickRoughness(NdV, F0, m.Roughness);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - m.Metallic;

    vec3 diffuse = m.Albedo * irradiance;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 R = reflect(-worldV, worldN);
    vec2 envBRDF = texture(u_BRDFLUT, vec2(NdV, m.Roughness)).rg;
    vec3 prefilteredColor = pow(textureLod(u_PrefilterMap, R, m.Roughness * MAX_REFLECTION_LOD).rgb, vec3(2.2));
    vec3 specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

    return (kD * diffuse + specular) * m.Ambient;
}*/


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
    return texCoords - (p * float(smoothstep(0, 1, u_EnableParallax)));
}

void main(){
    vec3 view_dir = normalize(TangentViewPos - TangentFragPos);
    vec2 tex_coords = ParallaxMapping(TexCoord, view_dir);
    
    Material m;
    m.Albedo = pow(texture(u_Albedo, tex_coords).rgb, vec3(GAMMA));
    m.Normal = FromNormalMap(tex_coords);
    m.Roughness = texture(u_RoughMetalAO, tex_coords).r;
    m.Metallic = texture(u_RoughMetalAO, tex_coords).g;
    m.Ambient = texture(u_SSAO, tex_coords).r;

   
    vec3 V = normalize(u_ViewPosition - WorldPos);
    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, m.Albedo, m.Metallic);

    vec3 Lo = vec3(0.0);

    float vis = shadowCalculationDirectional(WorldPos, directional.Direction, m.Normal);
    Lo += directionalLightRadiance(directional, m, F0, WorldPos, V) * (1 - vis);

    for(int i = 0; i < pointSize; i++){
        Lo += pointLightRadiance(point[i], m, F0, WorldPos, V);
    }

    vec3 kS = fresnelSchlick(max(dot(m.Normal, V), 0.0), F0);
    vec3 kD = 1.0 - kS;

    vec3 irradiance = texture(u_IrradianceMap, m.Normal).rgb;
    vec3 diffuse    = irradiance * m.Albedo;
    vec3 ambient    = (kD * diffuse) * m.Ambient; 
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/GAMMA)); 

    FragColor = vec4(color, 1.0);
    BrightColor = pow(texture(u_Emissive, TexCoord), vec4(GAMMA));
}