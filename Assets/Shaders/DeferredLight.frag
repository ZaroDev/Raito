#version 460 core

layout(location = 0) out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

in vec2 TexCoord;

struct DirLight {
    vec3 Position;
    vec3 Color;
    vec3 Direction;
};


struct PointLight {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};
const int NR_LIGHTS = 32;

uniform DirLight u_DirLights[NR_LIGHTS];
uniform PointLight u_PointLights[NR_LIGHTS];

uniform int u_DirLightsNum;
uniform int u_PointLightsNum;
uniform vec3 u_ViewPos;

vec3 CalcDirLight(DirLight light, vec3 diffuse, float specular, vec3 normal, vec3 fragPos, vec3 viewDir) {
    float distance = length(light.Position - fragPos);
    // diffuse
    vec3 lightDir = normalize(light.Position - fragPos);
    float r = length(lightDir);
    vec3 lightDiffuse = max(dot(normal, lightDir), 0.0) * diffuse * light.Color;
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    vec3 lightSpecular = light.Color * spec * specular;

    // spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light.Direction)); 
    //float epsilon = (0 - 0);
    float intensity = clamp((theta - 0) / 0.0, 0.0, 1.0);
    lightDiffuse  *= intensity;
    lightSpecular *= intensity;

    return (lightDiffuse + lightSpecular);
}

vec3 CalcLight(PointLight light, vec3 diffuse, float specular, vec3 normal, vec3 fragPos, vec3 viewDir){
    float distance = length(light.Position - fragPos);
    if(distance < light.Radius) {
        // Diffuse
        vec3 lightDir = normalize(light.Position - fragPos);
        vec3 lightDiffuse = max(dot(normal, lightDir), 0.0) * diffuse * light.Color;

        // Specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float lightSpec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
        vec3 lightSpecular = light.Color * lightSpec * specular;

        // Attenuation
        float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
        lightDiffuse *= attenuation;
        lightSpecular *= attenuation;

        return (lightDiffuse + lightSpecular);
    }

    return vec3(0.0);
}

void main() {
    vec3 FragPos = texture(gPosition, TexCoord).rgb;
    vec3 Normal = texture(gNormal, TexCoord).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoord).rgb;
    float Specular = texture(gAlbedoSpec, TexCoord).a;
    vec3 ViewDir  = normalize(u_ViewPos - FragPos);

    vec3 result = Diffuse * 0.1;

    for(int i = 0; i < u_DirLightsNum; i++) {
        result += CalcDirLight(u_DirLights[i], Diffuse, Specular, Normal, FragPos, ViewDir);
    }
    for(int i = 0; i < u_PointLightsNum; i++) {
        result += CalcLight(u_PointLights[i], Diffuse, Specular, Normal, FragPos, ViewDir);
    }
    

    FragColor = vec4(result, 1.0);
}