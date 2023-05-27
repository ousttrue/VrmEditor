#include "shader_source.h"
#include <ranges>
#include <span>
#include <string_view>
#include <vrm/fileutil.h>

namespace glr {

static auto error_vert = u8R"(#version 400
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
in vec3 vPosition;
void
main()
{
  gl_Position = Projection * View * Model * vec4(vPosition, 1.0);
}
)";

static auto error_frag = u8R"(#version 400
out vec4 FragColor;
void
main()
{
  FragColor = vec4(1, 0, 1, 1);
}
)";

// unlit.vert
static auto unlit_vert = u8R"(#version 400
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

// unlit.frag
static auto unlit_frag = u8R"(#version 400
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

// shadow.vert
static auto shadow_vert = u8R"(#version 400
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

// shadow.frag
static auto shadow_frag = u8R"(#version 400
in vec3 normal;
in vec2 uv;
out vec4 FragColor;
uniform sampler2D colorTexture;
void main()
{
  FragColor = vec4(0.3, 0.3, 0.3, 1.0);
};
)";

std::optional<std::filesystem::path>
get_include_path(std::u8string_view line)
{
  if (!line.starts_with(u8"#include ")) {
    return {};
  }
  auto tmp = line.substr(9);
  auto open = tmp.find(u8'"');
  if (open == std::string::npos) {
    return {};
  }
  tmp = tmp.substr(open + 1);
  auto close = tmp.find(u8'"');
  if (close == std::string::npos) {
    return {};
  }
  return tmp.substr(0, close);
}

static std::u8string
ExpandInclude(std::u8string_view src, const std::filesystem::path& dir)
{
  std::u8string dst;
  for (auto sv : src | std::views::split(u8'\n')) {
    std::u8string_view line{ sv };
    if (auto path = get_include_path(line)) {
      auto include = libvrm::fileutil::ReadAllBytes(dir / *path);
      dst +=
        std::u8string_view{ (const char8_t*)include.data(), include.size() };
    } else {
      dst += line;
    }
    dst.push_back('\n');
  }
  return dst;
}

struct ShaderSource
{
  std::filesystem::path Path;
  std::u8string Source;

  void Reload(const std::filesystem::path& dir)
  {
    auto path = dir / Path;
    if (std::filesystem::is_regular_file(path)) {
      auto bytes = libvrm::fileutil::ReadAllBytes(path);

      Source =
        ExpandInclude({ (const char8_t*)bytes.data(), bytes.size() }, dir);
    }
  }
};

struct ShaderSourceManagerImpl
{
  std::filesystem::path m_dir;
  std::vector<ShaderSource> m_sources = {
    ShaderSource{
      "error.vert",
      error_vert,
    },
    ShaderSource{
      "error.frag",
      error_frag,
    },
    ShaderSource{
      "pbr.vert",
    },
    ShaderSource{
      "pbr.frag",
    },
    ShaderSource{
      "unlit.vert",
      unlit_vert,
    },
    ShaderSource{
      "unlit.frag",
      unlit_frag,
    },
    ShaderSource{
      "shadow.vert",
      shadow_vert,
    },
    ShaderSource{
      "shadow.frag",
      shadow_frag,
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
