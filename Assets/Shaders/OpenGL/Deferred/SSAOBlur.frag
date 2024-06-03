#version 460 core
#extension GL_ARB_bindless_texture: require

layout(location = 0) out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D u_Texture;

void main() 
{
    vec2 texelSize = 1.0 / vec2(textureSize(u_Texture, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_Texture, TexCoords + offset).r;
        }
    }
    result = result / (4.0 * 4.0);
    FragColor = vec4(vec3(result), 1.0);
}  