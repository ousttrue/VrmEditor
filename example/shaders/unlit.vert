layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUv;
layout(location = 0) out vec3 normal;
layout(location = 1) out vec2 uv;

#include "ubo.glsl"

void
main()
{
  gl_Position = Env.projection * Env.view * Model.model * vec4(vPosition, 1.0);
  normal = vNormal;
  uv = vUv;
}
