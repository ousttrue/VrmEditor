#pragma once

class Gl3Renderer {

public:
  float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

  Gl3Renderer();
  ~Gl3Renderer();
  void clear(int width, int height);
  void render();
};
