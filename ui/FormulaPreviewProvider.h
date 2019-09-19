#ifndef CHAOSKIT_UI_FORMULAPREVIEWPROVIDER_H
#define CHAOSKIT_UI_FORMULAPREVIEWPROVIDER_H

#include <QQuickAsyncImageProvider>
#include <QThreadPool>

namespace chaoskit::ui {

class FormulaPreviewProvider : public QQuickAsyncImageProvider {
 public:
  QQuickImageResponse* requestImageResponse(
      const QString& id, const QSize& requestedSize) override;

 private:
  QThreadPool pool_;
};

}  // namespace chaoskit::ui

#endif  // CHAOSKIT_UI_FORMULAPREVIEWPROVIDER_H
