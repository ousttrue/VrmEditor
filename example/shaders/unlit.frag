layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;
layout(location = 0) out vec4 FragColor;

uniform sampler2D colorTexture;

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
  vec4 color = Model.color * texture(colorTexture, uv);
#ifdef MODE_MASK
  if (color.a < Model.cutoff.x) {
    discard;
  }
#endif
  // gamma correct
  color.rgb = pow(color.rgb, vec3(1.0 / 2.2));

  FragColor = color;
  // FragColor = vec4(normal, 1);
}
