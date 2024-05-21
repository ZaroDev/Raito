#version 460 core

layout(location = 0) out vec4 FragColor;

uniform sampler2D u_Diffuse;
uniform sampler2D u_Emissive;
uniform sampler2D u_Specular;


in vec2 TexCoord;

void main(){
    vec3 diffuse = texture(u_Diffuse, TexCoord).rgb;
    vec3 light = texture(u_Emissive, TexCoord).rbg;
    vec3 specular = texture(u_Specular, TexCoord).rgb;

    vec3 result = diffuse * 0.2;
    result += diffuse * light;
    result += specular;
    FragColor = vec4(result, 1.0);
}