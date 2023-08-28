#version 400
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
in vec3 vPosition;
in vec3 vNormal;
in vec2 vUv;
out vec3 gNormal;

void
main()
{
  gl_Position = Projection * View * Model * vec4(vPosition, 1.0);
  gNormal = vNormal;
}
