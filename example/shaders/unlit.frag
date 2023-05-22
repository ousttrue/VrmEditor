#version 400
in vec3 normal;
in vec2 uv;
out vec4 FragColor;
uniform vec4 color = vec4(1, 1, 1, 1);
uniform float cutoff = 1;
uniform sampler2D colorTexture;
void
main()
{
  vec4 texel = color * texture(colorTexture, uv);
  if (texel.a < cutoff) {
    discard;
  }
  FragColor = texel;
}
