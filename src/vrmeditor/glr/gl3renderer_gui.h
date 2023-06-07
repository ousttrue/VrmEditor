#pragma once
#include <TextEditor.h>

namespace glr {

struct Material;
class Gl3RendererGui
{
  TextEditor m_vsEditor;
  TextEditor m_fsEditor;
  uint32_t m_selected = 0;

public:
  void ShowShaderSource(Material& factory);
  void ShowShaderVariables(Material& factory);
  void Select(uint32_t i);

private:
  void ShowSelectedShaderSource();
  void ShowSelectedShaderVariables();
};

}
