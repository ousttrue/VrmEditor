#include "shader_source.h"
#include "app.h"
#include <plog/Log.h>
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
  return tmp.substr(0, close);
}

struct IncludeExpander
{
  std::filesystem::path ChunkDir;
  std::vector<std::filesystem::path> IncludeFiles;

  std::u8string GetOrExpandLine(const std::filesystem::path& base,
                                std::u8string_view line)
  {
    auto path = get_include_path(line);
    if (!path) {
      path = get_chunk_path(line);
    }
    if (path) {
      if (auto include = ExpandIncludeRecursive(base, *path)) {
        auto chunk = *include;

        // for threejs_shader_chunks
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
        PLOG_WARNING << include.error();
        return {};
      }
    } else {
      return { line.begin(), line.end() };
    }
  }

  std::expected<std::u8string, std::string> ExpandIncludeRecursive(
    const std::filesystem::path& base,
    const std::filesystem::path& include)
  {
    auto path = base / include;
    auto chunk = ChunkDir / (include.string() + ".glsl.js");
    if (std::filesystem::exists(path)) {
      return Expand(path);
    } else if (std::filesystem::exists(chunk)) {
      return Expand(chunk);
    } else {
      return std::unexpected{ "file not found: " + path.string() };
    }
  }

  std::expected<std::u8string, std::string> Expand(
    const std::filesystem::path& path)
  {
    auto found = std::find(IncludeFiles.begin(), IncludeFiles.end(), path);
    if (found != IncludeFiles.end()) {
      // ERROR
      return std::unexpected{ "circular include !" };
    }
    IncludeFiles.push_back(path);
    auto bytes = libvrm::ReadAllBytes(path);
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

  std::u8string ExpandInclude(const std::filesystem::path& path)
  {
    auto bytes = libvrm::ReadAllBytes(path);
    std::u8string_view root_source{ (const char8_t*)bytes.data(),
                                    bytes.size() };

    std::u8string dst;
    for (auto sv : root_source | std::views::split(u8'\n')) {
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
};

void
ShaderSource::Reload(const std::filesystem::path& dir,
                     const std::filesystem::path& chunkDir)
{
  auto path = dir / Path;
  if (std::filesystem::is_regular_file(path)) {
    IncludeExpander expander{ chunkDir };

    Source = expander.ExpandInclude(path);

#ifndef NDEBUG
    std::string debug((const char*)Source.data(), Source.size());
#endif

    for (auto& include : expander.IncludeFiles) {
      Includes.push_back(include.lexically_relative(dir));
    }
  }
}

struct ShaderSourceManagerImpl
{
  std::filesystem::path m_dir;
  std::filesystem::path m_chunkDir;
  std::vector<std::shared_ptr<ShaderSource>> m_sourceList;

  std::vector<std::filesystem::path> Update(const std::filesystem::path& path)
  {
    std::vector<std::filesystem::path> list;
    for (auto& source : m_sourceList) {
      if (Update(source, path)) {
        list.push_back(path.lexically_relative(m_dir));
      }
    }
    return list;
  }

  bool Update(const std::shared_ptr<ShaderSource>& source,
              const std::filesystem::path& path)
  {
    if (source->Path == path) {
      PLOG_INFO << source->Path.string() << ": updated";
      source->Source.clear();
      return true;
    } else {
      for (auto& include : source->Includes) {
        if (include == path) {
          PLOG_INFO << path.string() << " include from "
                    << source->Path.string() << ": updated";
          source->Source.clear();
          return true;
        }
      }
    }
    return false;
  }

  std::shared_ptr<ShaderSource> Get(const std::string& name)
  {
    std::shared_ptr<ShaderSource> source;
    for (auto& s : m_sourceList) {
      if (s->Path == name) {
        source = s;
        break;
      }
    }

    if (!source) {
      source = std::make_shared<ShaderSource>();
      m_sourceList.push_back(source);
      source->Path = name;
    }

    if (!m_dir.empty() && std::filesystem::exists(m_dir)) {
      source->Reload(m_dir, m_chunkDir);
    }

    return source;
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

std::shared_ptr<ShaderSource>
ShaderSourceManager::Get(const std::string& filename)
{
  return m_impl->Get(filename);
}

std::vector<std::filesystem::path>
ShaderSourceManager::UpdateShader(const std::filesystem::path& path)
{
  return m_impl->Update(path);
}

}
