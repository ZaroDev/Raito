#version 460 core

layout(location = 0) out vec4 FragColor;

in vec3 LocalPos;

uniform samplerCube u_EnvironmentMap;

void main() {
    vec3 envColor = texture(u_EnvironmentMap, LocalPos).rgb;
    FragColor = vec4(envColor, 1.0);
}