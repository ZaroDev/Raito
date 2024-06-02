#version 460 core
#extension GL_ARB_bindless_texture: require
out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D u_ScreenTexture;
layout(bindless_sampler) uniform sampler2D u_BloomTexture;

vec3 aces(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
  vec3 hdrColor = texture(u_ScreenTexture, TexCoords).rgb;
  vec3 bloomColor = texture(u_BloomTexture, TexCoords).rgb;

  vec3 result = hdrColor + bloomColor;
  result = aces(result);

  FragColor = vec4(result, 1.0);
}