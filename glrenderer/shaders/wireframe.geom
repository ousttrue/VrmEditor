#version 400
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 gNormal[];

out vec3 fNormal;
out vec3 fBaryxyz;

void
main()
{
  gl_Position = gl_in[0].gl_Position;
  fNormal = gNormal[0];
  fBaryxyz = vec3(1, 0, 0);
  EmitVertex();

  gl_Position = gl_in[1].gl_Position;
  fNormal = gNormal[1];
  fBaryxyz = vec3(0, 1, 0);
  EmitVertex();

  gl_Position = gl_in[2].gl_Position;
  fNormal = gNormal[2];
  fBaryxyz = vec3(0, 0, 1);
  EmitVertex();

  EndPrimitive();
}
