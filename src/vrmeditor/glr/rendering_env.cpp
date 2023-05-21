#include <GL/glew.h>

#include "app.h"
#include "rendering_env.h"
#include <grapho/gl3/pbr.h>
#include <vrm/fileutil.h>
#include <vrm/gltf.h>
#include <vrm/image.h>

namespace glr {

bool
RenderingEnv::LoadPbr(const std::filesystem::path& path)
{
  auto bytes = libvrm::fileutil::ReadAllBytes(path);
  if (bytes.empty()) {
    App::Instance().Log(LogLevel::Error) << "fail to read: " << path;
    return false;
  }

  auto hdr = std::make_shared<libvrm::gltf::Image>("pbr");
  if (!hdr->LoadHdr(bytes)) {
    App::Instance().Log(LogLevel::Error) << "fail to load: " << path;
    return false;
  }

  auto texture = grapho::gl3::Texture::Create(
    {
      .Width = hdr->Width(),
      .Height = hdr->Height(),
      .Format = grapho::PixelFormat::f32_RGB,
      .ColorSpace = grapho::ColorSpace::Linear,
      .Pixels = hdr->Pixels(),
    },
    true);
  if (!texture) {
    return false;
  }

  m_pbr = std::make_shared<grapho::gl3::PbrEnv>(texture);
  return true;
}

void
RenderingEnv::RenderSkybox()
{
  if (m_pbr) {
    m_pbr->DrawSkybox(ProjectionMatrix, ViewMatrix);
  }
}

}
