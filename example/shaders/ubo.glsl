layout(binding = 0, std140) uniform EnvVars
{
  mat4 projection;
  mat4 view;
  vec4 lightPositions[4];
  vec4 lightColors[4];
  vec4 camPos;
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
