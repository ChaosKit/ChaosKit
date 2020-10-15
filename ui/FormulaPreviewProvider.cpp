#include "FormulaPreviewProvider.h"

#include <QImage>
#include <QPainter>
#include <QQuickImageResponse>
#include <QRunnable>
#include <QSize>
#include <magic_enum.hpp>
#include <memory>
#include <unordered_set>
#include "ast/util.h"
#include "core/Params.h"
#include "core/SimpleInterpreter.h"
#include "core/structures/Blend.h"
#include "core/structures/Formula.h"
#include "core/structures/System.h"
#include "core/toSource.h"
#include "library/FormulaType.h"

namespace chaoskit::ui {

namespace {

constexpr int GRID_WIDTH = 16;
constexpr int GRID_HEIGHT = 16;
constexpr double GRID_EXPANSION_FACTOR = .2;
constexpr int ITERATIONS = 3;

/** AST visitor that checks if an expression contains a random number. */
class ContainsRandomNumber {
 public:
  bool operator()(const ast::RandomNumber&) const { return true; }

  bool operator()(const ast::UnaryFunction& fn) const {
    return ast::apply_visitor(*this, fn.argument());
  }

  bool operator()(const ast::BinaryFunction& fn) const {
    return ast::apply_visitor(*this, fn.first()) ||
           ast::apply_visitor(*this, fn.second());
  }

  bool operator()(const ast::VariableName& var) const {
    return variablesWithRandomNumbers_.find(var.name()) !=
           variablesWithRandomNumbers_.end();
  }

  template <typename T>
  bool operator()(const T&) const {
    return false;
  }

  void markVariable(std::string variableName) {
    variablesWithRandomNumbers_.insert(std::move(variableName));
  }

 private:
  std::unordered_set<std::string> variablesWithRandomNumbers_;
};

bool requiresScatterplot(library::FormulaType type) {
  auto formula = library::source(type);

  ContainsRandomNumber visitor;
  for (const auto& var : formula.variables()) {
    if (ast::apply_visitor(visitor, var.definition())) {
      visitor.markVariable(var.name());
    }
  }

  return ast::apply_visitor(visitor, formula.x()) ||
         ast::apply_visitor(visitor, formula.y());
}

core::SimpleInterpreter createInterpreter(library::FormulaType type) {
  auto formula = std::make_unique<core::Formula>();

  auto libraryFormula = library::createFormula(type);
  formula->type = type;
  formula->source = libraryFormula->source();
  formula->params = libraryFormula->exampleParams();

  auto blend = std::make_unique<core::Blend>();
  blend->pre = core::Transform(libraryFormula->examplePreTransform().params());
  blend->formulas.push_back(formula.get());

  core::System system{};
  system.blends.push_back(blend.get());

  return core::SimpleInterpreter(core::toSource(system),
                                 core::Particle::IMMORTAL, 0,
                                 core::Params::fromSystem(system));
}

QVector<QPointF> generateGrid(library::FormulaType type, int width,
                              int height) {
  core::SimpleInterpreter interpreter = createInterpreter(type);
  QVector<QPointF> points(width * height);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      // Convert the point to be in [-1; 1]
      core::Particle particle{{(float)(x * 2) / (float)(width - 1) - 1.f,
                               (float)(y * 2) / (float)(height - 1) - 1.f},
                              0.f,
                              core::Particle::IMMORTAL};

      int index = y * width + x;
      for (int i = 0; i < ITERATIONS; i++) {
        particle = interpreter(particle).next_state;
      }
      points[index].setX(particle.x());
      points[index].setY(particle.y());
    }
  }

  return points;
}

QRectF getBoundsForGrid(const QVector<QPointF>& grid) {
  auto top = std::numeric_limits<qreal>::max();
  auto right = std::numeric_limits<qreal>::min();
  auto bottom = std::numeric_limits<qreal>::min();
  auto left = std::numeric_limits<qreal>::max();

  for (const auto& point : grid) {
    if (point.x() < left) left = point.x();
    if (point.x() > right) right = point.x();
    if (point.y() < top) top = point.y();
    if (point.y() > bottom) bottom = point.y();
  }

  return QRectF(left, top, right - left, bottom - top);
}

class FormulaPreviewResponse : public QQuickImageResponse, public QRunnable {
 public:
  FormulaPreviewResponse(QString type, const QSize& requestedSize)
      : type_(std::move(type)), requestedSize_(requestedSize) {
    setAutoDelete(false);
  }

  void run() override {
    QSize imageSize(192, 192);

    image_ = QImage(imageSize, QImage::Format_RGBA64);
    image_.fill(QColor(0, 0, 0, 0));

    QPainter painter(&image_);
    QColor primary("#fde38a");
    painter.setPen(primary);

    auto formulaType =
        magic_enum::enum_cast<library::FormulaType>(type_.toStdString());
    if (formulaType) {
      QVector<QPointF> grid =
          generateGrid(*formulaType, GRID_WIDTH, GRID_HEIGHT);

      QRectF bounds = getBoundsForGrid(grid);
      double dx = bounds.width() * GRID_EXPANSION_FACTOR * .5;
      double dy = bounds.height() * GRID_EXPANSION_FACTOR * .5;
      bounds.adjust(-dx, -dy, dx, dy);

      QTransform transform =
          QTransform::fromScale(imageSize.width() / bounds.width(),
                                imageSize.height() / bounds.height())
              .translate(-bounds.left(), -bounds.top());

      if (requiresScatterplot(*formulaType)) {
        painter.setBrush(primary);
        QSize pointSize(imageSize.width() / (8 * GRID_WIDTH),
                        imageSize.height() / (8 * GRID_HEIGHT));
        QPoint offset(-pointSize.width() / 2, -pointSize.height() / 2);

        for (int y = 0; y < GRID_HEIGHT; y++) {
          for (int x = 0; x < GRID_WIDTH; x++) {
            int index = y * GRID_WIDTH + x;
            painter.drawRect(QRectF(transform.map(grid[index]), pointSize)
                                 .translated(offset));
          }
        }
      } else {
        for (int y = 0; y < GRID_HEIGHT; y++) {
          for (int x = 0; x < GRID_WIDTH; x++) {
            int index = y * GRID_WIDTH + x;

            if (x < GRID_WIDTH - 1) {
              int right = index + 1;
              painter.drawLine(transform.map(grid[index]),
                               transform.map(grid[right]));
            }
            if (y < GRID_HEIGHT - 1) {
              int bottom = index + GRID_WIDTH;
              painter.drawLine(transform.map(grid[index]),
                               transform.map(grid[bottom]));
            }
          }
        }
      }
    }

    if (requestedSize_.isValid()) {
      image_ = image_.scaled(requestedSize_);
    }

    emit finished();
  }

  [[nodiscard]] QQuickTextureFactory* textureFactory() const override {
    return QQuickTextureFactory::textureFactoryForImage(image_);
  }

 private:
  QString type_;
  QImage image_;
  QSize requestedSize_;
};

}  // namespace

QQuickImageResponse* FormulaPreviewProvider::requestImageResponse(
    const QString& id, const QSize& requestedSize) {
  auto* response = new FormulaPreviewResponse(id, requestedSize);
  pool_.start(response);
  return response;
}

}  // namespace chaoskit::ui
