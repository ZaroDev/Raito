#version 460 core
#extension GL_ARB_bindless_texture: require
out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler2D u_ScreenTexture;
layout(bindless_sampler) uniform sampler2D u_BloomTexture;

uniform int u_EnableBloom;
#define GAMMA 2.2



void main() {
  vec3 hdrColor = texture(u_ScreenTexture, TexCoords).rgb;
  vec3 bloomColor = texture(u_BloomTexture, TexCoords).rgb;

  vec3 imageColor = hdrColor + (bloomColor * smoothstep(0, 1, u_EnableBloom));
  imageColor *= 0.6;
	vec3 mapped = (imageColor * (2.51f * imageColor + 0.03f)) / (imageColor * (2.43f * imageColor + 0.59f) + 0.14f);

  FragColor = vec4(mapped, 1.0);
}