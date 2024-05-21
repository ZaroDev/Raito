#version 460 core


layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 SpecularColor;


layout(location = 0) uniform sampler2D u_Depth;
layout(location = 1) uniform sampler2D u_Normal;

uniform vec3 u_PixelSize;
uniform vec3 u_CameraPos;

uniform vec3 u_LightDirection;
uniform vec3 u_LightColor;



void main(){
    vec3 pos = vec3((gl_FragCoord.x * u_PixelSize.x),
                    (gl_FragCoord.y * u_PixelSize.y), 0.0);
    
    pos.z = texture(u_Depth, pos.xy).r;

    vec3 normal = normalize(texture(u_Normal, pos.xy).xyz * 2.0 - 1.0);

    vec3 lightDir = normalize(-u_LightDirection);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * u_LightColor;
    
    vec3 viewDir = normalize(u_CameraPos - pos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * u_LightColor;

    FragColor = vec4(diffuse, 1.0);
    SpecularColor = vec4(specular, 1.0);
}