#version 400
in vec3 fNormal;
in vec3 fBaryxyz;

out vec4 FragColor;

float
edgeFactor()
{
  vec3 d = fwidth(fBaryxyz);
  vec3 f = step(d * 0.5, fBaryxyz);
  return min(min(f.x, f.y), f.z);
}

void
main()
{
  float c = edgeFactor();
  FragColor = vec4(c, c, c, 1.0);
}
