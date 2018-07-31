#ifndef CHAOSKIT_UI_GLTONEMAPPER_H
#define CHAOSKIT_UI_GLTONEMAPPER_H

#include <core/HistogramBuffer.h>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLShaderProgram>

namespace chaoskit {
namespace ui {

class GLToneMapper : public QObject, protected QOpenGLFunctions_3_2_Core {
  Q_OBJECT
 public:
  explicit GLToneMapper(QObject *parent): QObject(parent) {}

  void initializeGL();
  void syncBuffer(const core::HistogramBuffer &buffer);
  void map();

 private:
  QOpenGLShaderProgram *program_ = nullptr;
  GLuint rectBuffer_ = 0;
  GLuint rectArray_ = 0;
  GLuint histogramTexture_ = 0;
  int positionAttribute_ = 0;
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_GLTONEMAPPER_H
