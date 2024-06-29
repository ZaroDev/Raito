#version 460 core
#extension GL_ARB_bindless_texture: require

#define PI 3.1415926
#define GAMMA 2.2

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;



layout(bindless_sampler) uniform sampler2D u_Directional;
layout(bindless_sampler) uniform sampler2D u_Point;
layout(bindless_sampler) uniform sampler2D u_Ambient;
layout(bindless_sampler) uniform sampler2D u_Emissive;

in vec2 TexCoord;

void main(){
    vec3 directional = texture(u_Directional, TexCoord).rgb;
    vec3 point = texture(u_Point, TexCoord).rgb;
    vec3 ambient = texture(u_Ambient, TexCoord).rgb;


    vec3 Lo = directional + point;
    vec3 color = (ambient * 0.1) + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/GAMMA)); 

    FragColor = vec4(color, 1.0);
    BrightColor = pow(texture(u_Emissive, TexCoord), vec4(GAMMA));
}