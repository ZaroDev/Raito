#version 460 core


layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 SpecularColor;


layout(location = 0) uniform sampler2D u_Depth;
layout(location = 1) uniform sampler2D u_Normal;

uniform vec3 u_PixelSize;
uniform vec3 u_CameraPos;
uniform vec3 u_LightColor;
uniform float u_LightRadius;

uniform vec3 u_LightPos;

in mat4 InverseProjView;

void main(){
    vec3 pos = vec3((gl_FragCoord.x * u_PixelSize.x),
                    (gl_FragCoord.y * u_PixelSize.y), 0.0);
    
    pos.z = texture(u_Depth, pos.xy).r;

    vec3 normal = normalize(texture(u_Normal, pos.xy).xyz * 2.0 - 1.0);

    vec4 clip = InverseProjView * vec4(pos * 2.0 - 1.0, 1.0);
    pos = clip.xyz / clip.w;

    float dist = length(u_LightPos - pos);
    float atten = 1.0 - clamp(dist / u_LightRadius, 0.0, 1.0);

    if(atten == 0.0){
        discard;
    }

    vec3 incident = normalize(u_LightPos - pos);
    vec3 viewDir = normalize(u_CameraPos - pos);
    vec3 halfDir = normalize(incident + viewDir);

    float lambert = clamp(dot(incident, normal), 0.0, 1.0);
    float rFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
    float sFactor = pow(rFactor, 33.0);

    FragColor = vec4(u_LightColor * lambert * atten, 1.0);
    SpecularColor = vec4(u_LightColor.xyz * sFactor * atten * 0.33, 1.0);
}