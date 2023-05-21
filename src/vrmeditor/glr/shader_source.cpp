#include "shader_source.h"
#include <vrm/fileutil.h>

namespace glr {

// unlit.vs
static auto vertex_shader_text = u8R"(#version 400
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
in vec3 vPosition;
in vec3 vNormal;
in vec2 vUv;
out vec3 normal;
out vec2 uv;
void main()
{
  gl_Position = Projection * View * Model * vec4(vPosition, 1.0);
  normal = vNormal;
  uv = vUv;
}
)";

// unlit.fs
static auto fragment_shader_text = u8R"(#version 400
in vec3 normal;
in vec2 uv;
out vec4 FragColor;
uniform vec4 color=vec4(1, 1, 1, 1);
uniform float cutoff=1;
uniform sampler2D colorTexture;
void main()
{
  vec4 texel = color * texture(colorTexture, uv);
  if(texel.a < cutoff)
  {
    discard;
  }
  FragColor = texel;
};
)";

// shadow.vs
static auto shadow_vertex_text = u8R"(#version 400
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 Shadow;
in vec3 vPosition;
in vec3 vNormal;
in vec2 vUv;
out vec3 normal;
out vec2 uv;
void main()
{
  gl_Position = Projection * View * Shadow * Model * vec4(vPosition, 1.0);
  normal = vNormal;
  uv = vUv;
}
)";

// shadow.fs
static auto shadow_fragment_text = u8R"(#version 400
in vec3 normal;
in vec2 uv;
out vec4 FragColor;
uniform sampler2D colorTexture;
void main()
{
  FragColor = vec4(0.3, 0.3, 0.3, 1.0);
};
)";

struct ShaderSource
{
  std::filesystem::path Path;
  std::u8string Source;

  void Reload(const std::filesystem::path& dir)
  {
    auto path = dir / Path;
    if (std::filesystem::is_regular_file(path)) {
      auto bytes = libvrm::fileutil::ReadAllBytes(path);
      Source = { bytes.begin(), bytes.end() };
    }
  }
};

struct ShaderSourceManagerImpl
{
  std::filesystem::path m_dir;
  std::vector<ShaderSource> m_sources = {
    ShaderSource{
      "pbr.vs",
    },
    ShaderSource{
      "pbr.fs",
    },
    ShaderSource{
      "unlit.vs",
      vertex_shader_text,
    },
    ShaderSource{
      "unlit.fs",
      fragment_shader_text,
    },
    ShaderSource{
      "shadow.vs",
      shadow_vertex_text,
    },
    ShaderSource{
      "shadow.fs",
      shadow_fragment_text,
    },
  };
  void Update(const std::filesystem::path& path)
  {
    for (auto& source : m_sources) {
      if (source.Path == path) {
        source.Source.clear();
      }
    }
  }
  std::u8string_view Get(const std::filesystem::path& path)
  {
    for (auto& source : m_sources) {
      if (source.Path == path) {
        if (!m_dir.empty() && std::filesystem::exists(m_dir)) {
          source.Reload(m_dir);
        }

        return source.Source;
      }
    }
    return {};
  }
};

ShaderSourceManager::ShaderSourceManager()
  : m_impl(new ShaderSourceManagerImpl)
{
}

ShaderSourceManager::~ShaderSourceManager()
{
  delete m_impl;
}

std::u8string_view
ShaderSourceManager::Get(const std::filesystem::path& path) const
{
  return m_impl->Get(path);
}

void
ShaderSourceManager::SetShaderDir(const std::filesystem::path& path)
{
  m_impl->m_dir = path;
}

void
ShaderSourceManager::UpdateShader(const std::filesystem::path& path)
{
  m_impl->Update(path);
}

}
