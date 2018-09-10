#include "GLToneMapper.h"

namespace chaoskit {
namespace ui {

namespace {

const char *VERTEX_SHADER = R"XD(
#version 150

const vec2 pointToUV = vec2(0.5, 0.5);

in vec2 position;
out vec2 uv;

void main() {
    uv = position * pointToUV + pointToUV;
    gl_Position = vec4(position, 0.0, 1.0);
}
)XD";

const char *FRAGMENT_SHADER = R"XD(
#version 150

in vec2 uv;
out vec4 outColor;
uniform sampler2D histogram;
uniform float gamma;
uniform float exposure;
uniform float vibrancy;

const float LOG10 = 2.302585092994046;
const float E = 2.718281828459045;

float logmap(float x) {
    return log(x + 1.0) / LOG10;
}

void main()
{
    vec4 point = texture(histogram, uv);
    float intensity = point.w * pow(E, -exposure + 1.0);

    float scale = logmap(intensity) / intensity;
    vec4 scaledPoint = point * scale;

    vec4 vibrantPoint = pow(scaledPoint, vec4(gamma));
    float gammaScale = vibrantPoint.w / scaledPoint.w;

    vec4 nonvibrantPoint = scaledPoint * gammaScale;

    vec3 newColor =
        (nonvibrantPoint + (vibrantPoint - nonvibrantPoint) * vibrancy).xyz;
    outColor = vec4(newColor, 1.0);
}
)XD";

}  // namespace

void GLToneMapper::getUniformLocation(const char *name, GLuint *output) {
  int location = program_->uniformLocation(name);
  if (location < 0) {
    qWarning() << "Invalid location for uniform:" << name;
  } else {
    *output = static_cast<GLuint>(location);
  }
}

void GLToneMapper::initializeGL() {
  initializeOpenGLFunctions();

  // Setup the rectangle buffer
  glGenBuffers(1, &rectBuffer_);

  static float rectVertices[] = {-1.0f, -1.0f, -1.0f, 1.0f,
                                 1.0f,  1.0f,  1.0f,  -1.0f};
  glBindBuffer(GL_ARRAY_BUFFER, rectBuffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), rectVertices,
               GL_STATIC_DRAW);

  // Setup textures
  glGenTextures(1, &histogramTexture_);

  // Setup shaders
  program_ = new QOpenGLShaderProgram(this);
  program_->addShaderFromSourceCode(QOpenGLShader::Vertex, VERTEX_SHADER);
  program_->addShaderFromSourceCode(QOpenGLShader::Fragment, FRAGMENT_SHADER);
  program_->link();

  getUniformLocation("gamma", &gammaLocation_);
  getUniformLocation("exposure", &exposureLocation_);
  getUniformLocation("vibrancy", &vibrancyLocation_);
  auto positionAttribute =
      static_cast<GLuint>(program_->attributeLocation("position"));

  glGenVertexArrays(1, &rectArray_);
  glBindVertexArray(rectArray_);
  glEnableVertexAttribArray(positionAttribute);
  glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void GLToneMapper::setGamma(float gamma) { gamma_ = gamma; }

void GLToneMapper::setExposure(float exposure) { exposure_ = exposure; }

void GLToneMapper::setVibrancy(float vibrancy) { vibrancy_ = vibrancy; }

void GLToneMapper::syncBuffer(const core::HistogramBuffer &buffer) {
  glBindTexture(GL_TEXTURE_2D, histogramTexture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, buffer.width(), buffer.height(), 0,
               GL_RGBA, GL_FLOAT, buffer.data());
}

void GLToneMapper::map() {
  glClearColor(0.f, 0.f, 0.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);

  program_->bind();
  program_->setUniformValue(gammaLocation_, gamma_);
  program_->setUniformValue(exposureLocation_, exposure_);
  program_->setUniformValue(vibrancyLocation_, vibrancy_);
  glBindVertexArray(rectArray_);
  glBindBuffer(GL_ARRAY_BUFFER, rectBuffer_);
  glBindTexture(GL_TEXTURE_2D, histogramTexture_);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  program_->release();
}

}  // namespace ui
}  // namespace chaoskit
