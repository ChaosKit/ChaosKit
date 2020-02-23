#ifndef CHAOSKIT_UI_COLORMAPPREVIEWPROVIDER_H
#define CHAOSKIT_UI_COLORMAPPREVIEWPROVIDER_H

#include <QQuickAsyncImageProvider>
#include <QThreadPool>
#include "ColorMapRegistry.h"

namespace chaoskit::ui {

class ColorMapPreviewProvider : public QQuickAsyncImageProvider {
 public:
  explicit ColorMapPreviewProvider(const ColorMapRegistry* colorMapRegistry)
      : colorMapRegistry_(colorMapRegistry) {}

  QQuickImageResponse* requestImageResponse(
      const QString& id, const QSize& requestedSize) override;

 private:
  const ColorMapRegistry* colorMapRegistry_;
  QThreadPool pool_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_COLORMAPPREVIEWPROVIDER_H
