#version 400
in vec3 normal;
in vec2 uv;
out vec4 FragColor;
uniform sampler2D colorTexture;
void
main()
{
  FragColor = vec4(0.3, 0.3, 0.3, 1.0);
};
