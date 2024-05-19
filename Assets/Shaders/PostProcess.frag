#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

layout(location = 0) uniform sampler2D u_ScreenTexture;

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
  vec3 result = aces(hdrColor);

  FragColor = vec4(result, 1.0);
}