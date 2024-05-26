#version 460 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D u_ScreenTexture;
uniform bool u_Horizontal;
uniform float u_Weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{             
    vec2 tex_offset = 1.0 / textureSize(u_ScreenTexture, 0); // gets size of single texel
    vec3 result = texture(u_ScreenTexture, TexCoords).rgb * u_Weight[0]; // current fragment's contribution
    if(u_Horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(u_ScreenTexture, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * u_Weight[i];
            result += texture(u_ScreenTexture, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * u_Weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(u_ScreenTexture, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * u_Weight[i];
            result += texture(u_ScreenTexture, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * u_Weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}