#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

layout(location = 0) uniform sampler2D u_ScreenTexture;
uniform vec2 u_Resolution;


void main() {
  vec3 results;
  vec2 texelSize = 1.0 / u_Resolution;
  float x = texelSize.x;
  float y = texelSize.y;
   // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(u_ScreenTexture, vec2(TexCoords.x - 2*x, TexCoords.y + 2*y)).rgb;
    vec3 b = texture(u_ScreenTexture, vec2(TexCoords.x,       TexCoords.y + 2*y)).rgb;
    vec3 c = texture(u_ScreenTexture, vec2(TexCoords.x + 2*x, TexCoords.y + 2*y)).rgb;

    vec3 d = texture(u_ScreenTexture, vec2(TexCoords.x - 2*x, TexCoords.y)).rgb;
    vec3 e = texture(u_ScreenTexture, vec2(TexCoords.x,       TexCoords.y)).rgb;
    vec3 f = texture(u_ScreenTexture, vec2(TexCoords.x + 2*x, TexCoords.y)).rgb;

    vec3 g = texture(u_ScreenTexture, vec2(TexCoords.x - 2*x, TexCoords.y - 2*y)).rgb;
    vec3 h = texture(u_ScreenTexture, vec2(TexCoords.x,       TexCoords.y - 2*y)).rgb;
    vec3 i = texture(u_ScreenTexture, vec2(TexCoords.x + 2*x, TexCoords.y - 2*y)).rgb;

    vec3 j = texture(u_ScreenTexture, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 k = texture(u_ScreenTexture, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;
    vec3 l = texture(u_ScreenTexture, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 m = texture(u_ScreenTexture, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1
    results = e*0.125;
    results += (a+c+g+i)*0.03125;
    results += (b+d+f+h)*0.0625;
    results += (j+k+l+m)*0.125;

  FragColor = vec4(results, 1.0);
}