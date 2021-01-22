#include "util.h"

#include <magic_enum.hpp>
#include <sstream>
#include "Blend.h"
#include "System.h"
#include "Transform.h"

namespace chaoskit::flame {

namespace {
class DebugStringGenerator {
 public:
  explicit DebugStringGenerator(int indentSize = 2)
      : indentSize_(indentSize), indent_(0), stream_() {
    stream_ << std::boolalpha;
  }

  std::string generate(const flame::System &system) {
    stream_ << "TTL: " << system.ttl << std::endl;

    stream_ << "Initial Transform: ";
    generate(system.initialTransform);
    stream_ << std::endl;

    stream_ << "Blends (" << system.blends.size() << ")" << std::endl;
    indent();
    int index = 0;
    for (const flame::Blend *blend : system.blends) {
      indentation();
      stream_ << "Blend " << index;

      if (blend == system.isolatedBlend) {
        stream_ << " (ISOLATED)";
      }

      stream_ << ":" << std::endl;
      indent();
      generate(*blend);
      outdent();
      index++;
    }
    outdent();

    stream_ << "Final Blend:" << std::endl;
    indent();
    generate(*system.finalBlend);
    outdent();

    return stream_.str();
  }

 private:
  void generate(const flame::BlendBase &blend) {
    indentation();
    stream_ << "Enabled: " << blend.enabled << std::endl;

    indentation();
    stream_ << "Pre: ";
    generate(blend.pre);
    stream_ << std::endl;

    indentation();
    stream_ << "Post: ";
    generate(blend.post);
    stream_ << std::endl;

    indentation();
    stream_ << "Coloring Method: ";
    generate(blend.coloringMethod);
    stream_ << std::endl;

    indentation();
    stream_ << "Formulas (" << blend.formulas.size() << ")" << std::endl;
    indent();
    int index = 0;
    for (const flame::Formula *formula : blend.formulas) {
      indentation();
      stream_ << "Formula " << index << ":" << std::endl;
      indent();
      generate(*formula);
      outdent();
      index++;
    }
    outdent();
  }
  void generate(const flame::Blend &blend) {
    indentation();
    stream_ << "Name: " << (blend.name.empty() ? "(empty)" : blend.name)
            << std::endl;

    indentation();
    stream_ << "Weight: " << blend.weight << std::endl;

    generate(static_cast<const flame::BlendBase &>(blend));
  }

  void generate(const flame::Formula &formula) {
    indentation();
    stream_ << "Type: "
            << magic_enum::enum_name<library::FormulaType>(formula.type)
            << std::endl;

    indentation();
    stream_ << "Weight: (" << formula.weight.x << ", " << formula.weight.y
            << ")" << std::endl;

    indentation();
    stream_ << "Params (" << formula.params.size() << "):" << std::endl;
    indent();
    auto oldPrecision =
        stream_.precision(std::numeric_limits<float>::max_digits10 + 1);
    for (auto param : formula.params) {
      indentation();
      stream_ << param << std::endl;
    }
    stream_.precision(oldPrecision);
    outdent();
  }

  void generate(const flame::Transform &transform) {
    for (auto value : transform.values) {
      stream_ << value << ' ';
    }
  }
  void generate(const flame::ColoringMethod &coloringMethod) {
    stream_ << magic_enum::enum_name<library::ColoringMethodType>(
        coloringMethod.type);
    for (auto param : coloringMethod.params) {
      stream_ << ' ' << param;
    }
  }

  void indent() { indent_ += indentSize_; }
  void outdent() { indent_ -= indentSize_; }
  void indentation() {
    for (int i = 0; i < indent_; i++) {
      stream_.put(' ');
    }
  }

  int indentSize_;
  int indent_;
  std::stringstream stream_;
};
}  // namespace

std::string debugString(const System &system) {
  return DebugStringGenerator().generate(system);
}

}  // namespace chaoskit::flame
