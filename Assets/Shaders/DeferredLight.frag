#version 460 core

layout(location = 0) out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

in vec2 TexCoord;

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};
const int NR_LIGHTS = 32;
uniform Light u_Lights[NR_LIGHTS];
uniform int u_LightsNum;
uniform vec3 u_ViewPos;

vec3 CalcLight(Light light, vec3 diffuse, float specular, vec3 normal, vec3 fragPos, vec3 viewDir){
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
    
    for(int i = 0; i < u_LightsNum; i++) {
        result += CalcLight(u_Lights[i], Diffuse, Specular, Normal, FragPos, ViewDir);
    }
    

    FragColor = vec4(result, 1.0);
}