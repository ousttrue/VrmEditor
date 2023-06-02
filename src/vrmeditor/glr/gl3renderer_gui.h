#pragma once
#include "material_factory.h"

class TextEditor;

namespace glr {

void
ShowShaderSource(MaterialFactory& factory,
                 TextEditor& vsEditor,
                 TextEditor& fsEditor);
void
ShowShaderVariables(MaterialFactory& factory);

}
