#pragma once

struct Camera {
  float viewport[4];
  float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};
  float view[16];
  float projection[16];

  void resize(int width, int height) {
    viewport[2] = static_cast<float>(width);
    viewport[3] = static_cast<float>(height);
  }
  int width() const { return static_cast<int>(viewport[2]); }
  int height() const { return static_cast<int>(viewport[3]); }

  float premul_r() const { return clear_color[0] * clear_color[3]; }
  float premul_g() const { return clear_color[1] * clear_color[3]; }
  float premul_b() const { return clear_color[2] * clear_color[3]; }
  float alpha() const { return clear_color[3]; }
};

class Gl3Renderer {

public:
  Gl3Renderer();
  ~Gl3Renderer();
  void clear();

  void render(const Camera &camera);
};
