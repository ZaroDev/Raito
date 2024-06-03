#version 330 core
#extension GL_ARB_bindless_texture: require

layout(location = 0) out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D u_Position;
layout(bindless_sampler) uniform sampler2D u_Normal;
layout(bindless_sampler) uniform sampler2D u_Noise;

struct Sample{
    vec3 Data;
};

uniform Sample u_Samples[64];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
uniform int u_KernelSize = 64;
float radius = 1.0;
float bias = 0.25;

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(1920.0/4.0, 1080.0/4.0); 

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
    // get input for SSAO algorithm
    vec3 fragPos = vec3(u_View * texture(u_Position, TexCoords)).rgb;
    vec3 normal = vec3(u_View * texture(u_Normal, TexCoords)).rgb;
    vec3 randomVec = texture(u_Noise, TexCoords * noiseScale).xyz;
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < u_KernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * u_Samples[i].Data; // from tangent to view-space
        samplePos = fragPos.xyz + samplePos * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = u_Projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = vec3(u_View * vec4(texture(u_Position, offset.xy).xyz, 1.0)).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / u_KernelSize);
    
    FragColor = vec4(vec3(occlusion), 1.0);
}