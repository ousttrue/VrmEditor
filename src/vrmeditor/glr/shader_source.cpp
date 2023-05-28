#include "shader_source.h"
#include "app.h"
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

std::u8string_view
SkipSpace(std::u8string_view src)
{
  auto it = src.begin();
  for (; it != src.end(); ++it) {
    if (*it != ' ') {
      break;
    }
  }
  return { it, src.end() };
}

std::optional<std::filesystem::path>
get_include_path(std::u8string_view line)
{
  line = SkipSpace(line);
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

std::optional<std::filesystem::path>
get_chunk_path(std::u8string_view line)
{
  line = SkipSpace(line);
  if (!line.starts_with(u8"#include ")) {
    return {};
  }
  auto tmp = line.substr(9);
  auto open = tmp.find(u8'<');
  if (open == std::string::npos) {
    return {};
  }
  tmp = tmp.substr(open + 1);
  auto close = tmp.find(u8'>');
  if (close == std::string::npos) {
    return {};
  }
  std::u8string chunk{ tmp.data(), close };
  return chunk + u8".glsl.js";
}

struct IncludeExpander
{
  std::filesystem::path Dir;
  std::filesystem::path ChunkDir;
  std::vector<std::filesystem::path> IncludeFiles;

  std::u8string GetOrExpandLine(const std::filesystem::path& base,
                                std::u8string_view line)
  {
    if (auto path = get_include_path(line)) {
      auto include = ExpandIncludeRecursive(base, *path);
      return include;
    } else if (auto path = get_chunk_path(line)) {
      auto chunk = ExpandIncludeRecursive(ChunkDir, *path);
      if (chunk.empty()) {
        App::Instance().Log(LogLevel::Wran)
          << "chunk: " << path->string() << " not found ?";
        return {};
      }
      const std::u8string EXPORT_DEFAULT = u8"export default /* glsl */`\n";
      if (chunk.starts_with(EXPORT_DEFAULT)) {
        chunk = chunk.substr(EXPORT_DEFAULT.size());
      }

      if (chunk.ends_with(u8"`;\n")) {
        chunk.pop_back();
        chunk.pop_back();
        chunk.pop_back();
      }
#ifndef NDEBUG
      std::string debug((const char*)chunk.data(), chunk.size());
      std::string debug_end = debug.substr(debug.size() - 10);
#endif
      return chunk;
    } else {
      return { line.begin(), line.end() };
    }
  }

  std::u8string ExpandIncludeRecursive(const std::filesystem::path& base,
                                       const std::filesystem::path& include)
  {
    auto path = base / include;
    auto found = std::find(IncludeFiles.begin(), IncludeFiles.end(), path);
    if (found != IncludeFiles.end()) {
      // ERROR
      return u8"circular include !";
    }

    IncludeFiles.push_back(path);
    auto bytes = libvrm::fileutil::ReadAllBytes(path);
    std::u8string_view source{ (const char8_t*)bytes.data(), bytes.size() };

    std::u8string dst;
    for (auto sv : source | std::views::split(u8'\n')) {
      std::u8string_view line{ sv };
      if (line.size() && line.back() == '\r') {
        line = line.substr(0, line.size() - 1);
      }
      if (line.size()) {
        dst += GetOrExpandLine(path.parent_path(), line);
        dst.push_back('\n');
      }
    }

    return dst;
  }

  std::u8string ExpandInclude(std::u8string root_source)
  {
    std::u8string dst;
    for (auto sv : root_source | std::views::split(u8'\n')) {
      std::u8string_view line{ sv };
      if (line.size() && line.back() == '\r') {
        line = line.substr(0, line.size() - 1);
      }
      if (line.size()) {
        dst += GetOrExpandLine(Dir, line);
        dst.push_back('\n');
      }
    }
    return dst;
  }
};

struct ShaderSource
{
  std::filesystem::path Path;
  std::u8string Source;
  std::vector<std::filesystem::path> Includes;

  void Reload(const std::filesystem::path& dir,
              const std::filesystem::path& chunkDir)
  {
    auto path = dir / Path;
    if (std::filesystem::is_regular_file(path)) {
      auto bytes = libvrm::fileutil::ReadAllBytes(path);

      IncludeExpander expander{ dir, chunkDir };

      Source =
        expander.ExpandInclude({ (const char8_t*)bytes.data(), bytes.size() });

#ifndef NDEBUG
      std::string debug((const char*)Source.data(), Source.size());
#endif

      for (auto& include : expander.IncludeFiles) {
        Includes.push_back(include.lexically_relative(dir));
      }
    }
  }
};

struct ShaderSourceManagerImpl
{
  std::filesystem::path m_dir;
  std::filesystem::path m_chunkDir;
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
    ShaderSource{
      "mtoon.vert",
      shadow_vert,
    },
    ShaderSource{
      "mtoon.frag",
      shadow_frag,
    },
  };
  std::vector<std::filesystem::path> Update(const std::filesystem::path& path)
  {
    std::vector<std::filesystem::path> list;
    for (auto& source : m_sources) {
      if (source.Path == path) {
        App::Instance().Log(LogLevel::Info) << source.Path << ": updated";
        source.Source.clear();
        list.push_back(source.Path);
      } else {
        for (auto& include : source.Includes) {
          if (include == path) {
            App::Instance().Log(LogLevel::Info)
              << path << " include from " << source.Path << ": updated";
            source.Source.clear();
            list.push_back(source.Path);
            break;
          }
        }
      }
    }
    return list;
  }
  std::u8string_view Get(const std::filesystem::path& path)
  {
    for (auto& source : m_sources) {
      if (source.Path == path) {
        if (!m_dir.empty() && std::filesystem::exists(m_dir)) {
          source.Reload(m_dir, m_chunkDir);
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
ShaderSourceManager::SetShaderChunkDir(const std::filesystem::path& path)
{
  m_impl->m_chunkDir = path;
}

std::vector<std::filesystem::path>
ShaderSourceManager::UpdateShader(const std::filesystem::path& path)
{
  return m_impl->Update(path);
}

}
