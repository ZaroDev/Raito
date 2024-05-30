#version 460 core

#define PI 3.1415926
#define MAX_LIGHTS 32

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

layout(location = 0) uniform sampler2D u_GBufferAlbedo;
layout(location = 1) uniform sampler2D u_GBufferNormal;
layout(location = 2) uniform sampler2D u_GBufferRoughness;
layout(location = 3) uniform sampler2D u_GBufferMetalness;
layout(location = 4) uniform sampler2D u_GBufferDepth;


uniform mat4x4 u_InvProjection;
uniform int u_NumLights;

in vec2 TexCoord;

struct Light{
  vec4 Position;
  vec3 Color;
};

uniform Light u_Lights[MAX_LIGHTS];


struct PBRMaterial{
    vec3 Albedo;
    vec3 Emissive;
    float Roughness;
    float Metalness;
    float Occlusion;
};


// Reconstruct view space position from depth
vec3 ViewPositionFromDepth(vec2 texcoord, float depth) {
  // Get x/w and y/w from the viewport position
  vec3 projectedPos = vec3(texcoord, depth) * 2.0f - 1.0f;;

  // Transform by the inverse projection matrix
  vec4 positionVS = u_InvProjection * vec4(projectedPos, 1.0f);

  // Divide by w to get the view-space position
  return positionVS.xyz / positionVS.w;
}

// Schlick-Frensel curve approximation
vec3 FresnelSchlick(vec3 F0, float cosTheta) {
   return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// NDF
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// G
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}



// PBR Direct lighting
vec3 DirectRadiance(Light light, vec3 P, vec3 N, vec3 V, PBRMaterial m, vec3 F0) {
  // Direction to light in viewspace
  vec3 L = normalize(light.Position.xyz - P);
  vec3 H = normalize(V + L);
  float distance = length(light.Position.xyz - P);
  float attenuation = 1.0 / (distance  * distance);
  vec3 radiance = light.Color * attenuation;

  float NDF = DistributionGGX(N, H, m.Roughness);
  float G = GeometrySmith(N, V, L, m.Roughness);
  vec3 F = FresnelSchlick( F0, max(dot(H, V), 0.0));

  vec3 kS = F;
  vec3 kD = vec3(1.0) - kS;
  kD *= 1.0 - m.Metalness;

  vec3 numerator = NDF * G * F;
  float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
  vec3 specular = numerator / denominator;

  float NdotL = max(dot(N, L), 0.0);
  return (kD * m.Albedo / PI + specular) * radiance * NdotL;
}


void main(){
    vec4 albedoAO = texture(u_GBufferAlbedo, TexCoord);
    vec4 emissive = texture(u_GBufferRoughness, TexCoord);
    vec3 N = normalize(texture(u_GBufferNormal, TexCoord).xyz);
    vec2 metalRoughness = texture(u_GBufferMetalness, TexCoord).yz;
    float D = texture(u_GBufferDepth, TexCoord).x;

    PBRMaterial m;
    m.Albedo    = albedoAO.rgb;
    m.Emissive  = emissive.rgb;
    m.Roughness = metalRoughness.y;
    m.Metalness = metalRoughness.x;
    m.Occlusion = albedoAO.w;

    // Recompute viewspace position from UV + depth
    vec3 P = ViewPositionFromDepth(TexCoord, D);

    // Direction to eye in viewspace
    vec3 V = normalize(-P);

    // Lerp between Dia-electric = 0.04f to Metal = albedo
    vec3 F0 = mix(vec3(0.04), m.Albedo, m.Metalness);



    vec3 ambient = vec3(0.03) * m.Albedo * m.Occlusion;
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < u_NumLights; i++) {
      Lo += DirectRadiance(u_Lights[i] ,P, N, V, m, F0);
    }

    vec3 result = ambient + Lo;
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0 / 2.2 )); 

    BrightColor = emissive;
    FragColor = vec4(result, 1.0);
  gl_FragDepth = D;
}