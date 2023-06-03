#pragma once
#include "material.h"

class TextEditor;

namespace glr {

void
ShowShaderSource(Material& factory,
                 TextEditor& vsEditor,
                 TextEditor& fsEditor);
void
ShowShaderVariables(Material& factory);

}
