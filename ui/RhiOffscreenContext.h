#ifndef CHAOSKIT_UI_RHIOFFSCREENCONTEXT_H
#define CHAOSKIT_UI_RHIOFFSCREENCONTEXT_H

#include <rhi/qrhi.h>
#include <QImage>
#include <QString>

#include <memory>

namespace chaoskit::ui {

// Converts a completed QRhi texture readback into an owning QImage, flipping
// vertically when the active backend uses Y-up framebuffers (OpenGL).
QImage imageFromRhiReadback(const QRhi* rhi, const QRhiReadbackResult& result);

// Owns a swapchain-free offscreen QRhi instance and any underlying native
// surface or Vulkan instance handles, probing platform backends in priority
// order without creating a window.
class RhiOffscreenContext {
 public:
  ~RhiOffscreenContext();

  // Non-copyable and non-movable: owns GPU/QRhi device and native surface
  // handles.
  RhiOffscreenContext(const RhiOffscreenContext&) = delete;
  RhiOffscreenContext& operator=(const RhiOffscreenContext&) = delete;
  RhiOffscreenContext(RhiOffscreenContext&&) = delete;
  RhiOffscreenContext& operator=(RhiOffscreenContext&&) = delete;

  static std::unique_ptr<RhiOffscreenContext> create(
      QString* errorMessage = nullptr);

  [[nodiscard]] QRhi* rhi() const { return rhi_.get(); }

 private:
  struct NativeHandles;

  RhiOffscreenContext();

  // Declared before rhi_ so rhi_ is destroyed first.
  std::unique_ptr<NativeHandles> nativeHandles_;
  std::unique_ptr<QRhi> rhi_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_RHIOFFSCREENCONTEXT_H
