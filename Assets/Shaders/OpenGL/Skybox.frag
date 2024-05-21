#version 460 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 LightColor;
in vec3 LocalPos;

uniform samplerCube u_EnvironmentMap;

void main() {
    vec3 envColor = texture(u_EnvironmentMap, LocalPos).rgb;

    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0 / 2.2));

    FragColor = vec4(envColor, 1.0);
    LightColor = vec4(vec3(0.0), 1.0);
}