layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

layout(location = 0) out vec3 Normal;
layout(location = 1) out vec2 TexCoords;
layout(location = 2) out vec3 WorldPos;

#include "ubo.glsl"

void
main()
{
  TexCoords = aTexCoords;
  WorldPos = vec3(Model.model * vec4(aPos, 1.0));
  Normal = mat3(Model.normalMatrix) * aNormal;

  gl_Position = Env.projection * Env.view * vec4(WorldPos, 1.0);
}
