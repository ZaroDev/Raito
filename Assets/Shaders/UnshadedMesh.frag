#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 u_ObjectColor;
uniform vec3 u_LightColor;

void main() 
{
    vec3 lightPos = vec3(0.0);
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * u_LightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);  

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_LightColor;

    vec3 result = (ambient + diffuse) * u_ObjectColor;

    FragColor = vec4(result, 1.0);
}