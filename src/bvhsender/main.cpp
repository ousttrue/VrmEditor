#include "BvhPanel.h"
#include "GlfwPlatform.h"
#include "GuiApp.h"
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>

int
main(int argc, char** argv)
{
  // imgui
  GuiApp app;

  // window
  GlfwPlatform platform;
  if (!platform.Create()) {
    return 1;
  }

  // bvh scene
  BvhPanel bvhPanel;

  // load bvh
  if (argc > 1) {
    if (auto bvh = libvrm::bvh::Bvh::FromFile(argv[1])) {
      bvhPanel.SetBvh(*bvh);
    }
  }
  platform.Callbacks.push_back([&bvhPanel](auto& path) mutable {
    if (auto bvh = libvrm::bvh::Bvh::FromFile(path)) {
      bvhPanel.SetBvh(*bvh);
    }
  });

  // cuber
  cuber::gl3::GlCubeRenderer cubeRenderer;
  cuber::gl3::GlLineRenderer lineRenderer;

  std::vector<cuber::LineVertex> lines;
  cuber::PushGrid(lines);

  // main loop
  while (auto time = platform.NewFrame(app.clear_color)) {
    // imgui
    {
      app.UpdateGui();
      bvhPanel.UpdateGui();
    }
    auto data = app.RenderGui();

    // scene
    {
      auto cubes = bvhPanel.GetCubes();
      cubeRenderer.Render(app.projection, app.view, cubes.data(), cubes.size());
      lineRenderer.Render(app.projection, app.view, lines);
      platform.EndFrame(data);
    }
  }

  return 0;
}
