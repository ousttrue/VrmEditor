layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUv;
layout(location = 0) out vec3 normal;
layout(location = 1) out vec2 uv;

layout(binding = 0, std140) uniform EnvVars
{
  mat4 projection;
  mat4 view;
  vec4 lightPositions[4];
  vec4 lightColors[4];
  vec3 camPos;
}
Env;

layout(binding = 1, std140) uniform ModelVars
{
  mat4 model;
  vec4 color;
  vec4 cutoff;
  mat4 normalMatrix;
}
Model;

void
main()
{
  gl_Position = Env.projection * Env.view * Model.model * vec4(vPosition, 1.0);
  normal = vNormal;
  uv = vUv;
}
