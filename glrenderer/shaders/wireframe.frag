#version 400
in vec3 normal;
in vec2 uv;
out vec4 FragColor;
uniform sampler2D colorTexture;

uniform vec3 dir = vec3(0, 1, 0);

float
edgeFactor()
{
  vec3 d = fwidth(baryxyz);
  vec3 f = step(d * lineWidth, baryxyz);
  return min(min(f.x, f.y), f.z);
}

void
main()
{
  FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
