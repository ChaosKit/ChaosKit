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
out vec3 outColor;
uniform sampler2D histogram;
uniform float gamma = 2.2;
uniform float exposure = 0.0;
uniform float vibrancy = 0.0;

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

    outColor =
        (nonvibrantPoint + (vibrantPoint - nonvibrantPoint) * vibrancy).xyz;
}
)XD";

}  // namespace

void GLToneMapper::initializeGL() {
  initializeOpenGLFunctions();

  // Setup the rectangle buffer
  glGenBuffers(1, &rectBuffer_);
  glGenVertexArrays(1, &rectArray_);
  static float rectVertices[] = {-1.0f, -1.0f, -1.0f, 1.0f,
                                 1.0f,  1.0f,  1.0f,  -1.0f};
  glBindVertexArray(rectArray_);
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
  positionAttribute_ = program_->attributeLocation("position");
}

void GLToneMapper::syncBuffer(const core::HistogramBuffer &buffer) {
  glBindTexture(GL_TEXTURE_2D, histogramTexture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, buffer.width(), buffer.height(), 0,
               GL_RGBA, GL_FLOAT, buffer.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void GLToneMapper::map() {
  glClear(GL_COLOR_BUFFER_BIT);

  program_->bind();
  glBindTexture(GL_TEXTURE_2D, histogramTexture_);
  glEnableVertexAttribArray(positionAttribute_);
  glVertexAttribPointer(positionAttribute_, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glDisableVertexAttribArray(positionAttribute_);
  program_->release();
}

}  // namespace ui
}  // namespace chaoskit
