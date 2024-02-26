#version 460 core
#extension GL_ARB_bindless_texture: require

layout(bindless_sampler) uniform sampler2D u_Diffuse;
layout(bindless_sampler) uniform sampler2D u_Normal;
layout(bindless_sampler) uniform sampler2D u_Emissive;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 u_ObjectColor;
uniform vec3 u_LightColor;
uniform vec3 u_ViewPos;

void main() 
{
    vec3 lightPos = vec3(0.0, 100.0, 100.0);
    float ambientStrength = 0.01;
    vec3 ambient = ambientStrength * u_LightColor;

    vec3 norm = texture(u_Normal, TexCoord).rgb;
    norm = normalize(norm * 2.0 - 1.0);

    vec3 lightDir = normalize(lightPos - FragPos);  

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_LightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(u_ViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * u_LightColor;  

    vec3 result = (ambient + diffuse + specular) * u_ObjectColor;

    FragColor = texture(u_Diffuse, TexCoord) * vec4(result, 1.0);

    // Check if the fragment output is higher than a threshold
    BrightColor = texture(u_Emissive, TexCoord);
}