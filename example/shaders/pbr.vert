layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

layout(location = 0) out vec3 Normal;
layout(location = 1) out vec2 TexCoords;
layout(location = 2) out vec3 WorldPos;

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

void
main()
{
  TexCoords = aTexCoords;
  WorldPos = vec3(Model.model * vec4(aPos, 1.0));
  Normal = mat3(Model.normalMatrix) * aNormal;

  gl_Position = Env.projection * Env.view * vec4(WorldPos, 1.0);
}
