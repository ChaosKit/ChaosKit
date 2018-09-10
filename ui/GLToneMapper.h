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
  explicit GLToneMapper(QObject *parent = nullptr): QObject(parent) {}

  void initializeGL();
  void setGamma(float gamma);
  void setExposure(float exposure);
  void setVibrancy(float vibrancy);
  void syncBuffer(const core::HistogramBuffer &buffer);
  void map();

 private:
  QOpenGLShaderProgram *program_ = nullptr;
  GLuint rectBuffer_ = 0;
  GLuint rectArray_ = 0;
  GLuint histogramTexture_ = 0;
  GLuint gammaLocation_ = 0;
  GLuint exposureLocation_ = 0;
  GLuint vibrancyLocation_ = 0;
  float gamma_ = 2.2f;
  float exposure_ = 0.f;
  float vibrancy_ = 0.f;

  void getUniformLocation(const char *name, GLuint *output);
};

}  // namespace ui
}  // namespace chaoskit

#endif  // CHAOSKIT_UI_GLTONEMAPPER_H
