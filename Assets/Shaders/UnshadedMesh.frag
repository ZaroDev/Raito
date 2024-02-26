#version 460 core
#extension GL_ARB_bindless_texture: require

layout(bindless_sampler) uniform sampler2D u_Texture;

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 u_ObjectColor;
uniform vec3 u_LightColor;
uniform vec3 u_ViewPos;

void main() 
{
    vec3 lightPos = vec3(1.0);
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * u_LightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);  

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_LightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(u_ViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  

    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * u_LightColor;  

    vec3 result = (ambient + diffuse + specular) * u_ObjectColor;

    FragColor = texture(u_Texture, TexCoord) * vec4(result, 1.0);
}