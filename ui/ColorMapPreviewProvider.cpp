#include "ColorMapPreviewProvider.h"
#include <QDebug>
#include "ColorMap.h"

namespace chaoskit::ui {

namespace {

class ColorMapResponse : public QQuickImageResponse, public QRunnable {
 public:
  ColorMapResponse(const core::ColorMap* colorMap, const QSize& requestedSize)
      : colorMap_(colorMap), requestedSize_(requestedSize) {
    setAutoDelete(false);
  }

  void run() override {
    if (requestedSize_.isValid()) {
      image_ = QImage(requestedSize_, QImage::Format_RGB32);
    } else {
      image_ = QImage(256, 1, QImage::Format_RGB32);
    }

    for (int x = 0; x < image_.width(); x++) {
      float value =
          static_cast<float>(x) / static_cast<float>(image_.width() - 1);
      auto color = colorMap_->map(value);
      auto qColor = QColor::fromRgbF(color.r, color.g, color.b);
      for (int y = 0; y < image_.height(); y++) {
        image_.setPixelColor(x, y, qColor);
      }
    }

    emit finished();
  }

  [[nodiscard]] QQuickTextureFactory* textureFactory() const override {
    return QQuickTextureFactory::textureFactoryForImage(image_);
  }

 private:
  const core::ColorMap* colorMap_;
  QSize requestedSize_;
  QImage image_;
};

}  // namespace

QQuickImageResponse* ColorMapPreviewProvider::requestImageResponse(
    const QString& id, const QSize& requestedSize) {
  auto* response =
      new ColorMapResponse(colorMapRegistry_->get(id), requestedSize);
  pool_.start(response);
  return response;
}

}  // namespace chaoskit::ui
