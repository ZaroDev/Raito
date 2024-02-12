#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D u_ScreenTexture;

vec4 exercise1(float coordX)
{
    return vec4(vec3(coordX), 1.0);
}

vec4 exercise2(vec2 uv)
{    
    return vec4(uv.yx, uv.x * uv.y, 1.0);
}

vec4 exercise3(vec2 uv)
{
    return exercise2(mod(uv, 1.0 / 5.0) * 5.0);
}

void main()
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = TexCoords/textureSize(u_ScreenTexture, 0);

    FragColor = texture(u_ScreenTexture, TexCoords) * exercise3(uv);
}