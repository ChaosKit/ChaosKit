#include "RhiOffscreenContext.h"

#include <rhi/qrhi_platform.h>
#include <QOffscreenSurface>
#include <QSize>
#include <QStringList>
#include <QVersionNumber>
#if QT_CONFIG(vulkan) && __has_include(<vulkan/vulkan.h>)
#include <QVulkanInstance>
#endif

#include <cstddef>
#include <memory>
#include <utility>

namespace chaoskit::ui {

struct RhiOffscreenContext::NativeHandles {
#if QT_CONFIG(vulkan) && __has_include(<vulkan/vulkan.h>)
  std::unique_ptr<QVulkanInstance> vulkanInstance;
#endif
  std::unique_ptr<QOffscreenSurface> glFallbackSurface;
};

namespace {

#if QT_CONFIG(metal)
bool tryCreateMetal(std::unique_ptr<QRhi>& rhiOut, QStringList& errors) {
  QRhiMetalInitParams params;
  if (!QRhi::probe(QRhi::Metal, &params)) {
    errors.append(QStringLiteral("Metal probe failed"));
    return false;
  }
  rhiOut.reset(QRhi::create(QRhi::Metal, &params));
  if (!rhiOut) {
    errors.append(QStringLiteral("Metal QRhi::create failed"));
    return false;
  }
  return true;
}
#endif

#if QT_CONFIG(vulkan) && __has_include(<vulkan/vulkan.h>)
bool tryCreateVulkan(std::unique_ptr<QVulkanInstance>& instanceOut,
                     std::unique_ptr<QRhi>& rhiOut, QStringList& errors) {
  auto instance = std::make_unique<QVulkanInstance>();
  const QVersionNumber supported = instance->supportedApiVersion();
  if (supported >= QVersionNumber(1, 1)) {
    instance->setApiVersion(QVersionNumber(1, 1));
  }
  instance->setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
  if (!instance->create()) {
    errors.append(QStringLiteral("QVulkanInstance::create failed"));
    return false;
  }

  QRhiVulkanInitParams params;
  params.inst = instance.get();
  if (!QRhi::probe(QRhi::Vulkan, &params)) {
    errors.append(QStringLiteral("Vulkan probe failed"));
    return false;
  }

  rhiOut.reset(QRhi::create(QRhi::Vulkan, &params));
  if (!rhiOut) {
    errors.append(QStringLiteral("Vulkan QRhi::create failed"));
    return false;
  }
  instanceOut = std::move(instance);
  return true;
}
#endif

#if defined(Q_OS_WIN)
bool tryCreateD3D11(std::unique_ptr<QRhi>& rhiOut, QStringList& errors) {
  QRhiD3D11InitParams params;
  if (!QRhi::probe(QRhi::D3D11, &params)) {
    errors.append(QStringLiteral("D3D11 probe failed"));
    return false;
  }
  rhiOut.reset(QRhi::create(QRhi::D3D11, &params));
  if (!rhiOut) {
    errors.append(QStringLiteral("D3D11 QRhi::create failed"));
    return false;
  }
  return true;
}

bool tryCreateD3D12(std::unique_ptr<QRhi>& rhiOut, QStringList& errors) {
  QRhiD3D12InitParams params;
  if (!QRhi::probe(QRhi::D3D12, &params)) {
    errors.append(QStringLiteral("D3D12 probe failed"));
    return false;
  }
  rhiOut.reset(QRhi::create(QRhi::D3D12, &params));
  if (!rhiOut) {
    errors.append(QStringLiteral("D3D12 QRhi::create failed"));
    return false;
  }
  return true;
}
#endif

#if QT_CONFIG(opengl)
bool tryCreateOpenGL(std::unique_ptr<QOffscreenSurface>& surfaceOut,
                     std::unique_ptr<QRhi>& rhiOut, QStringList& errors) {
  std::unique_ptr<QOffscreenSurface> surface(
      QRhiGles2InitParams::newFallbackSurface());
  if (!surface || !surface->isValid()) {
    errors.append(QStringLiteral("OpenGL fallback QOffscreenSurface failed"));
    return false;
  }

  QRhiGles2InitParams params;
  params.fallbackSurface = surface.get();
  if (!QRhi::probe(QRhi::OpenGLES2, &params)) {
    errors.append(QStringLiteral("OpenGL probe failed"));
    return false;
  }

  rhiOut.reset(QRhi::create(QRhi::OpenGLES2, &params));
  if (!rhiOut) {
    errors.append(QStringLiteral("OpenGL QRhi::create failed"));
    return false;
  }
  surfaceOut = std::move(surface);
  return true;
}
#endif

QImage copyConvertedReadback(const QRhi* rhi, const QRhiReadbackResult& result,
                             bool swapRedBlue) {
  const int width = result.pixelSize.width();
  const int height = result.pixelSize.height();
  const qsizetype bytesPerLine = static_cast<qsizetype>(width) * 4;
  const QImage view(reinterpret_cast<const uchar*>(result.data.constData()),
                    width, height, static_cast<int>(bytesPerLine),
                    QImage::Format_RGBA8888);
  QImage image = (rhi != nullptr && rhi->isYUpInFramebuffer())
                     ? view.flipped(Qt::Vertical)
                     : view.copy();
  if (swapRedBlue) {
    image = std::move(image).rgbSwapped();
  }
  return image;
}

}  // namespace

QImage imageFromRhiReadback(const QRhi* rhi, const QRhiReadbackResult& result) {
  if (result.pixelSize.isEmpty() || result.data.isEmpty()) {
    return {};
  }

  const qsizetype width = result.pixelSize.width();
  const qsizetype height = result.pixelSize.height();
  const qsizetype bytesPerLine = width * 4;
  if (result.data.size() < bytesPerLine * height) {
    return {};
  }

  bool swapRedBlue = false;
  switch (result.format) {
    case QRhiTexture::RGBA8:
      break;
    case QRhiTexture::BGRA8:
      swapRedBlue = true;
      break;
    default:
      return {};
  }

  return copyConvertedReadback(rhi, result, swapRedBlue);
}

RhiOffscreenContext::RhiOffscreenContext()
    : nativeHandles_(std::make_unique<NativeHandles>()) {}

RhiOffscreenContext::~RhiOffscreenContext() = default;

std::unique_ptr<RhiOffscreenContext> RhiOffscreenContext::create(
    QString* errorMessage) {
  std::unique_ptr<RhiOffscreenContext> context(new RhiOffscreenContext());
  QStringList errors;
  bool created = false;

#if defined(Q_OS_MACOS)
#if QT_CONFIG(metal)
  if (!created) {
    created = tryCreateMetal(context->rhi_, errors);
  }
#endif
#if QT_CONFIG(vulkan) && __has_include(<vulkan/vulkan.h>)
  if (!created) {
    created = tryCreateVulkan(context->nativeHandles_->vulkanInstance,
                              context->rhi_, errors);
  }
#endif
#if QT_CONFIG(opengl)
  if (!created) {
    created = tryCreateOpenGL(context->nativeHandles_->glFallbackSurface,
                              context->rhi_, errors);
  }
#endif
#elif defined(Q_OS_WIN)
  if (!created) {
    created = tryCreateD3D11(context->rhi_, errors);
  }
  if (!created) {
    created = tryCreateD3D12(context->rhi_, errors);
  }
#if QT_CONFIG(vulkan) && __has_include(<vulkan/vulkan.h>)
  if (!created) {
    created = tryCreateVulkan(context->nativeHandles_->vulkanInstance,
                              context->rhi_, errors);
  }
#endif
#if QT_CONFIG(opengl)
  if (!created) {
    created = tryCreateOpenGL(context->nativeHandles_->glFallbackSurface,
                              context->rhi_, errors);
  }
#endif
#else
#if QT_CONFIG(vulkan) && __has_include(<vulkan/vulkan.h>)
  if (!created) {
    created = tryCreateVulkan(context->nativeHandles_->vulkanInstance,
                              context->rhi_, errors);
  }
#endif
#if QT_CONFIG(opengl)
  if (!created) {
    created = tryCreateOpenGL(context->nativeHandles_->glFallbackSurface,
                              context->rhi_, errors);
  }
#endif
#endif

  if (!created || !context->rhi_) {
    if (errorMessage != nullptr) {
      *errorMessage = errors.isEmpty()
                          ? QStringLiteral("No supported QRhi backend found")
                          : errors.join(QStringLiteral("; "));
    }
    return nullptr;
  }

  return context;
}

}  // namespace chaoskit::ui
