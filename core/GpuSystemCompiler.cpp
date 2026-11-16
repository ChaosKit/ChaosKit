#include "GpuSystemCompiler.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "CameraSystem.h"
#include "Lifetime.h"
#include "TransformIndex.h"
#include "TransformParams.h"
#include "TransformSystem.h"
#include "ast/AffineTransform.h"
#include "ast/BinaryFunction.h"
#include "ast/Expression.h"
#include "ast/Formula.h"
#include "ast/Input.h"
#include "ast/MultiStepTransform.h"
#include "ast/Output.h"
#include "ast/Parameter.h"
#include "ast/RandomChoiceTransform.h"
#include "ast/RandomNumber.h"
#include "ast/Transform.h"
#include "ast/UnaryFunction.h"
#include "ast/VariableDeclaration.h"
#include "ast/VariableName.h"
#include "ast/WeightedSumTransform.h"
#include "ast/util.h"
#include "errors.h"

namespace chaoskit::core {

namespace {

struct UniformHeader {
  uint32_t width;
  uint32_t height;
  int32_t minLifetime;
  int32_t maxLifetime;

  uint32_t particleCount;
  uint32_t ticksPerInvocation;
  uint32_t seededMode;
  uint32_t reserved0;

  uint32_t seedLo;
  uint32_t seedHi;
  uint32_t tickOffsetLo;
  uint32_t tickOffsetHi;

  float halfMax;
  float halfWidth;
  float halfHeight;
  float reserved1;
};

static_assert(sizeof(UniformHeader) == GpuSystemCompiler::kHeaderByteSize);

std::string formatHexUint32(uint32_t value) {
  char buf[16];
  std::snprintf(buf, sizeof(buf), "%08x", value);
  return std::string(buf);
}

std::string formatGlslFloat(float value) {
  if (!std::isfinite(value)) {
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return "uintBitsToFloat(0x" + formatHexUint32(bits) + "u)";
  }
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%.9g", static_cast<double>(value));
  std::string out(buf);
  if (out == "-0") {
    out = "-0.0";
  } else if (out.find('.') == std::string::npos &&
             out.find('e') == std::string::npos &&
             out.find('E') == std::string::npos) {
    out += ".0";
  }
  if (!out.empty() && out.front() == '-') {
    return "(" + out + ")";
  }
  return out;
}

std::string uniformFloatExpr(uint32_t floatIndex) {
  static constexpr char kSwizzle[4] = {'x', 'y', 'z', 'w'};
  return "u.packedFloats[" + std::to_string(floatIndex / 4u) + "]." +
         kSwizzle[floatIndex % 4u];
}

void appendExpressionStructure(const ast::Expression& expr, std::string& out);

void appendTransformStructure(const ast::Transform& transform,
                              std::string& out);

class StructuralExpressionVisitor {
 public:
  explicit StructuralExpressionVisitor(std::string& out) : out_(out) {}

  void operator()(float value) const {
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    out_ += "n:" + formatHexUint32(bits);
  }

  void operator()(const ast::Input& input) const {
    out_ += "i:" + std::to_string(static_cast<int>(input.type()));
  }

  void operator()(const ast::Output& output) const {
    out_ += "o:" + std::to_string(static_cast<int>(output.type()));
  }

  void operator()(const ast::Parameter& param) const {
    out_ += "p:" + std::to_string(param.index());
  }

  void operator()(const ast::VariableName& var) const {
    out_ += "v:" + std::to_string(var.name().size()) + ":" + var.name();
  }

  void operator()(const ast::RandomNumber&) const { out_ += 'r'; }

  void operator()(const ast::UnaryFunction& fn) const {
    out_ += "u(" + std::to_string(static_cast<int>(fn.type())) + ",";
    appendExpressionStructure(fn.argument(), out_);
    out_ += ')';
  }

  void operator()(const ast::BinaryFunction& fn) const {
    out_ += "b(" + std::to_string(static_cast<int>(fn.type())) + ",";
    appendExpressionStructure(fn.first(), out_);
    out_ += ',';
    appendExpressionStructure(fn.second(), out_);
    out_ += ')';
  }

 private:
  std::string& out_;
};

void appendExpressionStructure(const ast::Expression& expr, std::string& out) {
  ast::apply_visitor(StructuralExpressionVisitor(out), expr);
}

class StructuralTransformVisitor {
 public:
  explicit StructuralTransformVisitor(std::string& out) : out_(out) {}

  void operator()(const ast::AffineTransform&) const { out_ += 'A'; }

  void operator()(const ast::Formula& formula) const {
    out_ += "F(" + std::to_string(formula.variables().size());
    for (const auto& decl : formula.variables()) {
      out_ +=
          ';' + std::to_string(decl.name().size()) + ':' + decl.name() + '=';
      appendExpressionStructure(decl.definition(), out_);
    }
    out_ += ";x=";
    appendExpressionStructure(formula.x(), out_);
    out_ += ";y=";
    appendExpressionStructure(formula.y(), out_);
    out_ += ')';
  }

  void operator()(const ast::MultiStepTransform& transform) const {
    appendMulti('M', transform.transforms());
  }

  void operator()(const ast::RandomChoiceTransform& transform) const {
    appendMulti('R', transform.transforms());
  }

  void operator()(const ast::WeightedSumTransform& transform) const {
    appendMulti('W', transform.transforms());
  }

 private:
  void appendMulti(char tag,
                   const std::vector<ast::Transform>& children) const {
    out_ += tag;
    out_ += '(' + std::to_string(children.size());
    for (const auto& child : children) {
      out_ += ':';
      appendTransformStructure(child, out_);
    }
    out_ += ')';
  }

  std::string& out_;
};

void appendTransformStructure(const ast::Transform& transform,
                              std::string& out) {
  ast::apply_visitor(StructuralTransformVisitor(out), transform.variant());
  out += ";c(";
  appendExpressionStructure(transform.color(), out);
  out += ')';
}

struct ParamKey {
  bool isCamera;
  TransformIndex index;
  size_t paramIndex;

  bool operator==(const ParamKey& other) const {
    return isCamera == other.isCamera && index == other.index &&
           paramIndex == other.paramIndex;
  }
};

struct ParamKeyHash {
  size_t operator()(const ParamKey& key) const noexcept {
    size_t h = key.index.hash();
    h ^= std::hash<size_t>{}(key.paramIndex) + 0x9e3779b9U + (h << 6U) +
         (h >> 2U);
    if (key.isCamera) {
      h ^= 0x517cc1b727220a95ULL;
    }
    return h;
  }
};

class ShaderBuilder {
 public:
  explicit ShaderBuilder(UniformLayout& layout) : layout_(layout) {}

  uint32_t allocAffine(bool isCamera, const TransformIndex& index) {
    const uint32_t offset = layout_.packedFloatCount;
    layout_.affineSlots.push_back({index, offset, isCamera});
    layout_.packedFloatCount += 6u;
    return offset;
  }

  uint32_t allocParam(bool isCamera, const TransformIndex& index,
                      size_t paramIndex) {
    const ParamKey key{isCamera, index, paramIndex};
    auto it = paramMap_.find(key);
    if (it != paramMap_.end()) {
      return it->second;
    }
    const uint32_t offset = layout_.packedFloatCount;
    layout_.paramSlots.push_back({index, paramIndex, offset, isCamera});
    paramMap_.emplace(key, offset);
    layout_.packedFloatCount += 1u;
    return offset;
  }

  uint32_t allocRandomChoice(bool isCamera, const TransformIndex& index,
                             size_t branchCount) {
    const uint32_t offset = layout_.packedFloatCount;
    if (branchCount > 0) {
      layout_.randomChoiceSlots.push_back(
          {index, branchCount, offset, isCamera});
      layout_.packedFloatCount += static_cast<uint32_t>(branchCount);
    }
    return offset;
  }

  uint32_t allocWeightedSum(bool isCamera, const TransformIndex& index,
                            size_t branchCount) {
    const uint32_t offset = layout_.packedFloatCount;
    if (branchCount > 0) {
      layout_.weightedSumSlots.push_back(
          {index, branchCount, offset, isCamera});
      layout_.packedFloatCount += static_cast<uint32_t>(branchCount);
    }
    return offset;
  }

  std::string nextTemp(const char* prefix) {
    return std::string(prefix) + "_" + std::to_string(tempCounter_++);
  }

  void emitTransform(std::ostringstream& code, const std::string& indent,
                     const ast::Transform& transform, const std::string& pIn,
                     const std::string& pOut, const TransformIndex& index,
                     bool isCamera);

 private:
  struct ExprContext {
    const TransformIndex& index;
    bool isCamera;
    bool allowOutput;
    const std::string& pIn;
    const std::string& pOut;
    const std::string& validVar;
    const std::string& rngVar;
    const std::unordered_map<std::string, std::string>& variables;
  };

  std::string emitExpression(std::ostringstream& code,
                             const std::string& indent,
                             const ast::Expression& expr,
                             const ExprContext& ctx);

  UniformLayout& layout_;
  std::unordered_map<ParamKey, uint32_t, ParamKeyHash> paramMap_;
  uint64_t tempCounter_ = 0;

  class ExprVisitor {
   public:
    ExprVisitor(ShaderBuilder& builder, std::ostringstream& code,
                const std::string& indent, const ExprContext& ctx)
        : builder_(builder), code_(code), indent_(indent), ctx_(ctx) {}

    std::string operator()(float value) const { return formatGlslFloat(value); }

    std::string operator()(const ast::Input& input) const {
      switch (input.type()) {
        case ast::Input::X:
          return ctx_.pIn + ".x";
        case ast::Input::Y:
          return ctx_.pIn + ".y";
        case ast::Input::COLOR:
          return ctx_.pIn + ".z";
      }
      return "0.0";
    }

    std::string operator()(const ast::Output& output) const {
      if (!ctx_.allowOutput) {
        throw OutputNotAvailable();
      }
      switch (output.type()) {
        case ast::Output::X:
          return ctx_.pOut + ".x";
        case ast::Output::Y:
          return ctx_.pOut + ".y";
        case ast::Output::COLOR:
          return ctx_.pOut + ".z";
      }
      return "0.0";
    }

    std::string operator()(const ast::Parameter& param) const {
      const uint32_t slot =
          builder_.allocParam(ctx_.isCamera, ctx_.index, param.index());
      return uniformFloatExpr(slot);
    }

    std::string operator()(const ast::VariableName& var) const {
      auto it = ctx_.variables.find(var.name());
      if (it == ctx_.variables.end()) {
        throw UndefinedVariableError(var.name());
      }
      return it->second;
    }

    std::string operator()(const ast::RandomNumber&) const {
      std::string tmp = builder_.nextTemp("rnd");
      code_ << indent_ << "float " << tmp << " = ck_rand01(" << ctx_.rngVar
            << ");\n";
      return tmp;
    }

    std::string operator()(const ast::UnaryFunction& fn) const {
      const std::string arg =
          builder_.emitExpression(code_, indent_, fn.argument(), ctx_);
      const std::string tmp = builder_.nextTemp("e");
      code_ << indent_ << "float " << tmp << " = ";
      switch (fn.type()) {
        case ast::UnaryFunction::SIN:
          code_ << "sin(" << arg << ")";
          break;
        case ast::UnaryFunction::COS:
          code_ << "cos(" << arg << ")";
          break;
        case ast::UnaryFunction::TAN:
          code_ << "tan(" << arg << ")";
          break;
        case ast::UnaryFunction::MINUS:
          code_ << "-(" << arg << ")";
          break;
        case ast::UnaryFunction::SQRT:
          code_ << "ck_sqrt(" << arg << ", " << ctx_.validVar << ")";
          break;
        case ast::UnaryFunction::ATAN:
          code_ << "atan(" << arg << ")";
          break;
        case ast::UnaryFunction::TRUNC:
          code_ << "trunc(" << arg << ")";
          break;
        case ast::UnaryFunction::EXP:
          code_ << "exp(" << arg << ")";
          break;
        case ast::UnaryFunction::FLOOR:
          code_ << "floor(" << arg << ")";
          break;
        case ast::UnaryFunction::CEIL:
          code_ << "ceil(" << arg << ")";
          break;
        case ast::UnaryFunction::SIGNUM:
          code_ << "ck_signum(" << arg << ")";
          break;
        case ast::UnaryFunction::ABS:
          code_ << "abs(" << arg << ")";
          break;
        case ast::UnaryFunction::NOT:
          code_ << "((" << arg << ") == 0.0 ? 1.0 : 0.0)";
          break;
        case ast::UnaryFunction::FRAC:
          code_ << "ck_frac(" << arg << ", " << ctx_.validVar << ")";
          break;
        case ast::UnaryFunction::SINH:
          code_ << "sinh(" << arg << ")";
          break;
        case ast::UnaryFunction::COSH:
          code_ << "cosh(" << arg << ")";
          break;
      }
      code_ << ";\n";
      return tmp;
    }

    std::string operator()(const ast::BinaryFunction& fn) const {
      const std::string first =
          builder_.emitExpression(code_, indent_, fn.first(), ctx_);
      const std::string second =
          builder_.emitExpression(code_, indent_, fn.second(), ctx_);
      const std::string tmp = builder_.nextTemp("e");
      code_ << indent_ << "float " << tmp << " = ";
      switch (fn.type()) {
        case ast::BinaryFunction::ADD:
          code_ << "(" << first << " + " << second << ")";
          break;
        case ast::BinaryFunction::SUBTRACT:
          code_ << "(" << first << " - " << second << ")";
          break;
        case ast::BinaryFunction::MULTIPLY:
          code_ << "(" << first << " * " << second << ")";
          break;
        case ast::BinaryFunction::DIVIDE:
          code_ << "ck_div(" << first << ", " << second << ", " << ctx_.validVar
                << ")";
          break;
        case ast::BinaryFunction::POWER:
          code_ << "ck_pow(" << first << ", " << second << ", " << ctx_.validVar
                << ")";
          break;
        case ast::BinaryFunction::MODULO:
          code_ << "ck_mod(" << first << ", " << second << ", " << ctx_.validVar
                << ")";
          break;
        case ast::BinaryFunction::AND:
          code_ << "(((" << first << " != 0.0) && (" << second
                << " != 0.0)) ? 1.0 : 0.0)";
          break;
        case ast::BinaryFunction::OR:
          code_ << "(((" << first << " != 0.0) || (" << second
                << " != 0.0)) ? 1.0 : 0.0)";
          break;
        case ast::BinaryFunction::LESS_THAN:
          code_ << "((" << first << " < " << second << ") ? 1.0 : 0.0)";
          break;
        case ast::BinaryFunction::GREATER_THAN:
          code_ << "((" << first << " > " << second << ") ? 1.0 : 0.0)";
          break;
        case ast::BinaryFunction::EQUALS:
          code_ << "((" << first << " == " << second << ") ? 1.0 : 0.0)";
          break;
        case ast::BinaryFunction::LESS_THAN_OR_EQUAL:
          code_ << "((" << first << " <= " << second << ") ? 1.0 : 0.0)";
          break;
        case ast::BinaryFunction::GREATER_THAN_OR_EQUAL:
          code_ << "((" << first << " >= " << second << ") ? 1.0 : 0.0)";
          break;
        case ast::BinaryFunction::DISTANCE:
          code_ << "abs(" << first << " - " << second << ")";
          break;
        case ast::BinaryFunction::ATAN2:
          code_ << "ck_atan2(" << first << ", " << second << ", "
                << ctx_.validVar << ")";
          break;
      }
      code_ << ";\n";
      return tmp;
    }

   private:
    ShaderBuilder& builder_;
    std::ostringstream& code_;
    const std::string& indent_;
    const ExprContext& ctx_;
  };

  class TransformVisitor {
   public:
    TransformVisitor(ShaderBuilder& builder, std::ostringstream& code,
                     const std::string& indent, const std::string& pIn,
                     const std::string& varOut, const TransformIndex& index,
                     bool isCamera)
        : builder_(builder),
          code_(code),
          indent_(indent),
          pIn_(pIn),
          varOut_(varOut),
          index_(index),
          isCamera_(isCamera) {}

    void operator()(const ast::AffineTransform&) const {
      const uint32_t base = builder_.allocAffine(isCamera_, index_);
      code_ << indent_ << varOut_ << " = vec3(\n"
            << indent_ << "    " << uniformFloatExpr(base + 0u) << " * " << pIn_
            << ".x + " << uniformFloatExpr(base + 1u) << " * " << pIn_
            << ".y + " << uniformFloatExpr(base + 2u) << ",\n"
            << indent_ << "    " << uniformFloatExpr(base + 3u) << " * " << pIn_
            << ".x + " << uniformFloatExpr(base + 4u) << " * " << pIn_
            << ".y + " << uniformFloatExpr(base + 5u) << ",\n"
            << indent_ << "    " << pIn_ << ".z);\n";
    }

    void operator()(const ast::Formula& formula) const {
      std::unordered_map<std::string, std::string> variables;
      const std::string emptyOut;
      const std::string validVar = isCamera_ ? "camValid" : "sysValid";
      const std::string rngVar = "rng";
      ExprContext ctx = {index_,   isCamera_, false,  pIn_,
                         emptyOut, validVar,  rngVar, variables};

      for (const auto& decl : formula.variables()) {
        const std::string defExpr =
            builder_.emitExpression(code_, indent_, decl.definition(), ctx);
        const std::string varIdent = builder_.nextTemp("fvar");
        code_ << indent_ << "float " << varIdent << " = " << defExpr << ";\n";
        variables.emplace(decl.name(), varIdent);
      }

      const std::string fx =
          builder_.emitExpression(code_, indent_, formula.x(), ctx);
      const std::string fy =
          builder_.emitExpression(code_, indent_, formula.y(), ctx);
      code_ << indent_ << varOut_ << " = vec3(" << fx << ", " << fy << ", "
            << pIn_ << ".z);\n";
    }

    void operator()(const ast::MultiStepTransform& transform) const {
      const auto& children = transform.transforms();
      if (children.empty()) {
        code_ << indent_ << varOut_ << " = " << pIn_ << ";\n";
        return;
      }

      std::string cur = builder_.nextTemp("msCur");
      code_ << indent_ << "vec3 " << cur << " = " << pIn_ << ";\n";
      TransformIndex childIndex = index_.firstChild();
      for (const auto& step : children) {
        const std::string stepOut = builder_.nextTemp("msOut");
        code_ << indent_ << "vec3 " << stepOut << ";\n";
        builder_.emitTransform(code_, indent_, step, cur, stepOut, childIndex,
                               isCamera_);
        cur = stepOut;
        childIndex = childIndex.nextSibling();
      }
      code_ << indent_ << varOut_ << " = " << cur << ";\n";
    }

    void operator()(const ast::RandomChoiceTransform& transform) const {
      const size_t count = transform.size();
      if (count == 0) {
        code_ << indent_ << varOut_ << " = " << pIn_ << ";\n";
        return;
      }

      const uint32_t base =
          builder_.allocRandomChoice(isCamera_, index_, count);
      code_ << indent_ << varOut_ << " = " << pIn_ << ";\n";
      code_ << indent_ << "if (";
      for (size_t i = 0; i < count; ++i) {
        if (i > 0) {
          code_ << " || ";
        }
        code_ << "(" << uniformFloatExpr(base + static_cast<uint32_t>(i))
              << " >= 0.0)";
      }
      code_ << ") {\n";

      const std::string innerIndent = indent_ + "    ";
      const std::string branchIndent = innerIndent + "    ";
      const std::string rollVar = builder_.nextTemp("rcRoll");
      code_ << innerIndent << "float " << rollVar << " = ck_rand01(rng);\n";

      TransformIndex childIndex = index_.firstChild();
      for (size_t i = 0; i < count; ++i) {
        code_ << innerIndent << (i == 0 ? "if (" : "else if (") << rollVar
              << " <= " << uniformFloatExpr(base + static_cast<uint32_t>(i))
              << ") {\n";
        builder_.emitTransform(code_, branchIndent, transform.transforms()[i],
                               pIn_, varOut_, childIndex, isCamera_);
        code_ << innerIndent << "}\n";
        childIndex = childIndex.nextSibling();
      }
      code_ << indent_ << "}\n";
    }

    void operator()(const ast::WeightedSumTransform& transform) const {
      const size_t count = transform.size();
      if (count == 0) {
        code_ << indent_ << varOut_ << " = " << pIn_ << ";\n";
        return;
      }

      const uint32_t base = builder_.allocWeightedSum(isCamera_, index_, count);
      code_ << indent_ << varOut_ << " = vec3(0.0, 0.0, 0.0);\n";
      const std::string branchIndent = indent_ + "    ";
      TransformIndex childIndex = index_.firstChild();
      for (size_t i = 0; i < count; ++i) {
        const std::string branchOut = builder_.nextTemp("wsOut");
        code_ << indent_ << "{\n";
        code_ << branchIndent << "vec3 " << branchOut << ";\n";
        builder_.emitTransform(code_, branchIndent, transform.transforms()[i],
                               pIn_, branchOut, childIndex, isCamera_);
        code_ << branchIndent << varOut_ << " += " << branchOut << " * "
              << uniformFloatExpr(base + static_cast<uint32_t>(i)) << ";\n";
        code_ << indent_ << "}\n";
        childIndex = childIndex.nextSibling();
      }
    }

   private:
    ShaderBuilder& builder_;
    std::ostringstream& code_;
    const std::string& indent_;
    const std::string& pIn_;
    const std::string& varOut_;
    const TransformIndex& index_;
    bool isCamera_;
  };
};

std::string ShaderBuilder::emitExpression(std::ostringstream& code,
                                          const std::string& indent,
                                          const ast::Expression& expr,
                                          const ExprContext& ctx) {
  return ast::apply_visitor(ExprVisitor(*this, code, indent, ctx), expr);
}

void ShaderBuilder::emitTransform(std::ostringstream& code,
                                  const std::string& indent,
                                  const ast::Transform& transform,
                                  const std::string& pIn,
                                  const std::string& pOut,
                                  const TransformIndex& index, bool isCamera) {
  const std::string varOut = nextTemp("vout");
  code << indent << "vec3 " << varOut << ";\n";
  ast::apply_visitor(
      TransformVisitor(*this, code, indent, pIn, varOut, index, isCamera),
      transform.variant());

  const auto* outColor = transform.color().match(
      [](const ast::Output& o) -> const ast::Output* { return &o; },
      [](const auto&) -> const ast::Output* { return nullptr; });
  if (outColor != nullptr && outColor->type() == ast::Output::COLOR) {
    code << indent << pOut << " = " << varOut << ";\n";
    return;
  }

  const std::unordered_map<std::string, std::string> emptyVars;
  const std::string validVar = isCamera ? "camValid" : "sysValid";
  const std::string rngVar = "rng";
  ExprContext colorCtx = {index,  isCamera, true,   pIn,
                          varOut, validVar, rngVar, emptyVars};
  const std::string colorExpr =
      emitExpression(code, indent, transform.color(), colorCtx);
  code << indent << pOut << " = vec3(" << varOut << ".x, " << varOut << ".y, "
       << colorExpr << ");\n";
}

class WeightCollector {
 public:
  WeightCollector(bool isCamera, const UniformLayout& layout, size_t& rcCursor,
                  size_t& wsCursor, std::vector<float>& packed)
      : isCamera_(isCamera),
        layout_(layout),
        rcCursor_(rcCursor),
        wsCursor_(wsCursor),
        packed_(packed) {}

  void collectTransform(const ast::Transform& transform,
                        const TransformIndex& index) {
    transform.variant().match(
        [](const ast::AffineTransform&) {}, [](const ast::Formula&) {},
        [&](const ast::MultiStepTransform& ms) {
          TransformIndex child = index.firstChild();
          for (const auto& step : ms.transforms()) {
            collectTransform(step, child);
            child = child.nextSibling();
          }
        },
        [&](const ast::RandomChoiceTransform& rc) {
          const size_t count = rc.size();
          if (count > 0) {
            if (rcCursor_ >= layout_.randomChoiceSlots.size()) {
              throw std::invalid_argument(
                  "UniformLayout RandomChoiceSlot count mismatch");
            }
            const auto& slot = layout_.randomChoiceSlots[rcCursor_++];
            if (slot.isCamera != isCamera_ || slot.index != index ||
                slot.branchCount != count) {
              throw std::invalid_argument(
                  "UniformLayout RandomChoiceSlot mismatch");
            }
            packRandomChoiceWeights(rc.weights(), count, slot.floatOffset);
          }
          TransformIndex child = index.firstChild();
          for (const auto& branch : rc.transforms()) {
            collectTransform(branch, child);
            child = child.nextSibling();
          }
        },
        [&](const ast::WeightedSumTransform& ws) {
          const size_t count = ws.size();
          if (count > 0) {
            if (wsCursor_ >= layout_.weightedSumSlots.size()) {
              throw std::invalid_argument(
                  "UniformLayout WeightedSumSlot count mismatch");
            }
            const auto& slot = layout_.weightedSumSlots[wsCursor_++];
            if (slot.isCamera != isCamera_ || slot.index != index ||
                slot.branchCount != count) {
              throw std::invalid_argument(
                  "UniformLayout WeightedSumSlot mismatch");
            }
            const auto& weights = ws.weights();
            for (size_t i = 0; i < count; ++i) {
              packed_[slot.floatOffset + i] = static_cast<float>(weights[i]);
            }
          }
          TransformIndex child = index.firstChild();
          for (const auto& branch : ws.transforms()) {
            collectTransform(branch, child);
            child = child.nextSibling();
          }
        });
  }

 private:
  void packRandomChoiceWeights(const std::vector<double>& weights, size_t count,
                               uint32_t floatOffset) {
    double total = 0.0;
    size_t lastPositive = count;
    for (size_t i = 0; i < count; ++i) {
      const double w = (std::max)(0.0, weights[i]);
      if (w > 0.0) {
        total += w;
        lastPositive = i;
      }
    }

    if (total <= 0.0 || lastPositive == count || !std::isfinite(total)) {
      for (size_t i = 0; i < count; ++i) {
        packed_[floatOffset + i] = -1.0f;
      }
      return;
    }

    double running = 0.0;
    for (size_t i = 0; i < count; ++i) {
      const double w = (std::max)(0.0, weights[i]);
      if (w <= 0.0) {
        packed_[floatOffset + i] = -1.0f;
      } else if (i == lastPositive) {
        packed_[floatOffset + i] = 2.0f;
      } else {
        running += w;
        packed_[floatOffset + i] = static_cast<float>(running / total);
      }
    }
  }

  bool isCamera_;
  const UniformLayout& layout_;
  size_t& rcCursor_;
  size_t& wsCursor_;
  std::vector<float>& packed_;
};

constexpr char kShaderPreamble[] = R"GLSL(#version 450

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct GpuParticle {
    vec4 posColor;
    ivec4 lifeWarmupRng;
};

)GLSL";

constexpr char kShaderHelpers[] = R"GLSL(
layout(std430, binding = 1) buffer ParticleBuffer {
    GpuParticle particles[];
} particlePool;

layout(std430, binding = 2) buffer HistogramBuffer {
    uint words[];
} histogram;

layout(std430, binding = 3) buffer CounterBuffer {
    uint plottedLo;
    uint plottedHi;
    uint pad0;
    uint pad1;
} counter;

layout(binding = 4) uniform sampler2D colorMapLut;

shared uint s_plotted[64];

bool ck_isfinite(float x) {
    return (floatBitsToUint(x) & 0x7f800000u) != 0x7f800000u;
}

float ck_signum(float x) {
    return ((floatBitsToUint(x) & 0x80000000u) != 0u) ? -1.0 : 1.0;
}

float ck_sqrt(float x, inout bool valid) {
    if (x < 0.0 || !ck_isfinite(x)) {
        valid = false;
        return 0.0;
    }
    return sqrt(x);
}

float ck_div(float a, float b, inout bool valid) {
    if (b == 0.0 || !ck_isfinite(a) || !ck_isfinite(b)) {
        valid = false;
        return 0.0;
    }
    float res = a / b;
    if (!ck_isfinite(res)) {
        valid = false;
        return 0.0;
    }
    return res;
}

float ck_frac(float x, inout bool valid) {
    if (!ck_isfinite(x)) {
        valid = false;
        return 0.0;
    }
    return x - trunc(x);
}

float ck_mod(float a, float b, inout bool valid) {
    if (!ck_isfinite(a) || !ck_isfinite(b) || b == 0.0) {
        valid = false;
        return 0.0;
    }
    return a - b * trunc(a / b);
}

float ck_atan2(float y, float x, inout bool valid) {
    if (!ck_isfinite(y) || !ck_isfinite(x)) {
        valid = false;
        return 0.0;
    }
    if (y == 0.0 && x == 0.0) {
        return 0.0;
    }
    return atan(y, x);
}

float ck_pow(float base, float exponent, inout bool valid) {
    if (!ck_isfinite(base) || !ck_isfinite(exponent)) {
        valid = false;
        return 0.0;
    }
    if (exponent == 0.0) {
        return 1.0;
    }
    if (base == 0.0) {
        if (exponent < 0.0) {
            valid = false;
            return 0.0;
        }
        return 0.0;
    }
    if (base < 0.0) {
        if (trunc(exponent) != exponent) {
            valid = false;
            return 0.0;
        }
        float mag = pow(-base, exponent);
        if (!ck_isfinite(mag)) {
            valid = false;
            return 0.0;
        }
        bool odd = abs(exponent - 2.0 * trunc(exponent * 0.5)) > 0.5;
        return odd ? -mag : mag;
    }
    float res = pow(base, exponent);
    if (!ck_isfinite(res)) {
        valid = false;
        return 0.0;
    }
    return res;
}

void ck_umul32(uint a, uint b, out uint hi, out uint lo) {
    uint a0 = a & 0xffffu;
    uint a1 = a >> 16u;
    uint b0 = b & 0xffffu;
    uint b1 = b >> 16u;
    uint p00 = a0 * b0;
    uint p01 = a0 * b1;
    uint p10 = a1 * b0;
    uint p11 = a1 * b1;
    uint mid = (p00 >> 16u) + (p01 & 0xffffu) + (p10 & 0xffffu);
    lo = (p00 & 0xffffu) | ((mid & 0xffffu) << 16u);
    hi = p11 + (p01 >> 16u) + (p10 >> 16u) + (mid >> 16u);
}

uvec4 ck_philox4x32(uvec4 ctr, uvec2 key) {
    for (int r = 0; r < 10; ++r) {
        uint hi0;
        uint lo0;
        uint hi1;
        uint lo1;
        ck_umul32(0xd2511f53u, ctr.x, hi0, lo0);
        ck_umul32(0xcd9e8d57u, ctr.z, hi1, lo1);
        ctr = uvec4(hi1 ^ ctr.y ^ key.x, lo1, hi0 ^ ctr.w ^ key.y, lo0);
        key.x += 0x9e3779b9u;
        key.y += 0xbb67ae85u;
    }
    return ctr;
}

struct CkRng {
    uvec4 ctr;
    uvec2 key;
    uvec4 pool;
    uint cursor;
    uint tickLo;
    uint tickHi;
    uint drawSubIndex;
};

uint ck_lane(uvec4 v, uint lane) {
    if (lane == 0u) return v.x;
    if (lane == 1u) return v.y;
    if (lane == 2u) return v.z;
    return v.w;
}

uint ck_next_u32(inout CkRng rng) {
    if (u.seededMode != 0u) {
        uint block = rng.drawSubIndex >> 2u;
        uint lane = rng.drawSubIndex & 3u;
        if (rng.cursor != block) {
            rng.pool = ck_philox4x32(
                uvec4(gl_GlobalInvocationID.x, rng.tickLo, rng.tickHi, block),
                rng.key);
            rng.cursor = block;
        }
        rng.drawSubIndex += 1u;
        return ck_lane(rng.pool, lane);
    }
    if (rng.cursor >= 4u) {
        rng.ctr.x += 1u;
        if (rng.ctr.x == 0u) {
            rng.ctr.y += 1u;
        }
        rng.pool = ck_philox4x32(rng.ctr, rng.key);
        rng.cursor = 0u;
    }
    uint word = ck_lane(rng.pool, rng.cursor);
    rng.cursor += 1u;
    return word;
}

float ck_rand01(inout CkRng rng) {
    return float(ck_next_u32(rng) >> 8u) * 5.9604644775390625e-8;
}

void ck_atomicAdd64(uint wordIdx, uint val) {
    if (val == 0u) {
        return;
    }
    uint old = atomicAdd(histogram.words[wordIdx], val);
    if (old + val < old) {
        atomicAdd(histogram.words[wordIdx + 1u], 1u);
    }
}
)GLSL";

}  // namespace

std::string GpuSystemCompiler::computeStructuralKey(
    const CameraSystem& cameraSystem) {
  std::string key = "S(";
  appendTransformStructure(cameraSystem.system.transform, key);
  key += ");C(";
  if (cameraSystem.camera) {
    appendTransformStructure(cameraSystem.camera->transform, key);
  } else {
    key += "none";
  }
  key += ')';
  return key;
}

CompiledGpuSystem GpuSystemCompiler::compile(const CameraSystem& cameraSystem) {
  CompiledGpuSystem compiled;
  compiled.structuralKey = computeStructuralKey(cameraSystem);
  compiled.layout.structuralKey = compiled.structuralKey;

  std::ostringstream systemCode;
  std::ostringstream cameraCode;
  ShaderBuilder builder(compiled.layout);

  builder.emitTransform(systemCode, "        ", cameraSystem.system.transform,
                        "sysIn", "sysOut", TransformIndex(), false);
  if (cameraSystem.camera) {
    builder.emitTransform(cameraCode, "        ",
                          cameraSystem.camera->transform, "camIn", "camOut",
                          TransformIndex(), true);
  }

  const uint32_t packedVec4Count =
      (std::max)(1u, (compiled.layout.packedFloatCount + 3u) / 4u);
  compiled.layout.packedVec4Count = packedVec4Count;
  compiled.layout.uniformBufferSize = kHeaderByteSize + packedVec4Count * 16u;

  std::ostringstream shader;
  shader << kShaderPreamble;
  shader << "layout(std140, binding = 0) uniform SystemParams {\n"
         << "    uint width;\n"
         << "    uint height;\n"
         << "    int minLifetime;\n"
         << "    int maxLifetime;\n"
         << "    uint particleCount;\n"
         << "    uint ticksPerInvocation;\n"
         << "    uint seededMode;\n"
         << "    uint reserved0;\n"
         << "    uint seedLo;\n"
         << "    uint seedHi;\n"
         << "    uint tickOffsetLo;\n"
         << "    uint tickOffsetHi;\n"
         << "    float halfMax;\n"
         << "    float halfWidth;\n"
         << "    float halfHeight;\n"
         << "    float reserved1;\n"
         << "    vec4 packedFloats[" << packedVec4Count << "];\n"
         << "} u;\n\n";
  shader << kShaderHelpers << "\n";

  shader << R"GLSL(void main() {
    uint localPlotted = 0u;
    uint gid = gl_GlobalInvocationID.x;
    if (gid < u.particleCount) {
        GpuParticle gp = particlePool.particles[gid];
        vec3 p = gp.posColor.xyz;
        int lifetime = gp.lifeWarmupRng.x;
        int warmup = gp.lifeWarmupRng.y;

        CkRng rng;
        rng.ctr = uvec4(uint(gp.lifeWarmupRng.z), uint(gp.lifeWarmupRng.w), gid, 0u);
        rng.key = (u.seededMode != 0u)
                      ? uvec2(u.seedLo, u.seedHi)
                      : uvec2(u.seedLo ^ gid, u.seedHi ^ (gid * 0x9e3779b9u));
        rng.pool = uvec4(0u);
        rng.cursor = (u.seededMode != 0u) ? 0xffffffffu : 4u;
        rng.tickLo = u.tickOffsetLo;
        rng.tickHi = u.tickOffsetHi;
        rng.drawSubIndex = 0u;

        for (uint tick = 0u; tick < u.ticksPerInvocation; ++tick) {
            if (u.seededMode != 0u) {
                rng.tickLo = u.tickOffsetLo + tick;
                rng.tickHi = u.tickOffsetHi + (rng.tickLo < u.tickOffsetLo ? 1u : 0u);
                rng.drawSubIndex = 0u;
                rng.cursor = 0xffffffffu;
            }

            if (lifetime == 0) {
                p.x = ck_rand01(rng) * 2.0 - 1.0;
                p.y = ck_rand01(rng) * 2.0 - 1.0;
                p.z = ck_rand01(rng);
                lifetime = u.maxLifetime;
                warmup = u.minLifetime;
            }

            vec3 sysIn = p;
            vec3 sysOut;
            bool sysValid = true;
)GLSL";

  shader << systemCode.str();

  shader << R"GLSL(
            if (!sysValid || !ck_isfinite(sysOut.x) || !ck_isfinite(sysOut.y) ||
                !ck_isfinite(sysOut.z)) {
                p = sysOut;
                lifetime = 0;
                warmup = 0;
                continue;
            }

            p = sysOut;
            if (lifetime != -1) {
                lifetime -= 1;
            }
            if (warmup > 0) {
                warmup -= 1;
                continue;
            }

)GLSL";

  if (cameraSystem.camera) {
    shader << "            vec3 camIn = p;\n"
           << "            vec3 camOut;\n"
           << "            bool camValid = true;\n"
           << cameraCode.str()
           << "            if (!camValid || !ck_isfinite(camOut.x) || "
              "!ck_isfinite(camOut.y) ||\n"
           << "                !ck_isfinite(camOut.z)) {\n"
           << "                continue;\n"
           << "            }\n";
  } else {
    shader << "            vec3 camOut = p;\n";
  }

  shader << R"GLSL(
            localPlotted += 1u;

            float fx = camOut.x * u.halfMax + u.halfWidth;
            float fy = camOut.y * u.halfMax + u.halfHeight;
            if (fx >= 0.0 && fy >= 0.0 && fx < float(u.width) &&
                fy < float(u.height)) {
                uint ix = uint(fx);
                uint iy = uint(fy);
                if (ix < u.width && iy < u.height) {
                    float lutCoord =
                        (clamp(camOut.z, 0.0, 1.0) * 1023.0 + 0.5) / 1024.0;
                    vec4 rgba = clamp(
                        textureLod(colorMapLut, vec2(lutCoord, 0.5), 0.0),
                        vec4(0.0), vec4(1.0));
                    uint base = (iy * u.width + ix) * 8u;
                    uint rFixed = uint(round(rgba.r * 16777216.0));
                    uint gFixed = uint(round(rgba.g * 16777216.0));
                    uint bFixed = uint(round(rgba.b * 16777216.0));
                    ck_atomicAdd64(base + 0u, rFixed);
                    ck_atomicAdd64(base + 2u, gFixed);
                    ck_atomicAdd64(base + 4u, bFixed);
                    ck_atomicAdd64(base + 6u, 16777216u);
                }
            }
        }

        particlePool.particles[gid].posColor = vec4(p, 0.0);
        particlePool.particles[gid].lifeWarmupRng = ivec4(
            lifetime, warmup, int(rng.ctr.x), int(rng.ctr.y));
    }

    s_plotted[gl_LocalInvocationIndex] = localPlotted;
    barrier();
    for (uint stride = 32u; stride > 0u; stride >>= 1u) {
        if (gl_LocalInvocationIndex < stride) {
            s_plotted[gl_LocalInvocationIndex] +=
                s_plotted[gl_LocalInvocationIndex + stride];
        }
        barrier();
    }
    if (gl_LocalInvocationIndex == 0u) {
        uint wgTotal = s_plotted[0];
        if (wgTotal > 0u) {
            uint old = atomicAdd(counter.plottedLo, wgTotal);
            if (old + wgTotal < old) {
                atomicAdd(counter.plottedHi, 1u);
            }
        }
    }
}
)GLSL";

  compiled.computeShaderSource = shader.str();
  return compiled;
}

std::vector<uint8_t> GpuSystemCompiler::packUniforms(
    const UniformLayout& layout, const CameraSystem& cameraSystem,
    const GpuDispatchConfig& config) {
  if (computeStructuralKey(cameraSystem) != layout.structuralKey) {
    throw std::invalid_argument(
        "CameraSystem structure does not match UniformLayout");
  }

  const uint32_t vec4Count = (std::max)(1u, layout.packedVec4Count);
  std::vector<uint8_t> buffer(kHeaderByteSize + vec4Count * 16u, 0);

  UniformHeader header = {};
  header.width = config.width;
  header.height = config.height;
  header.minLifetime = cameraSystem.lifetime.min();
  header.maxLifetime = cameraSystem.lifetime.max();
  header.particleCount = config.particleCount;
  header.ticksPerInvocation = config.ticksPerInvocation;
  header.seededMode = config.seededMode ? 1u : 0u;
  header.reserved0 = 0u;
  header.seedLo = static_cast<uint32_t>(config.seed & 0xffffffffULL);
  header.seedHi = static_cast<uint32_t>(config.seed >> 32u);
  header.tickOffsetLo =
      static_cast<uint32_t>(config.tickOffset & 0xffffffffULL);
  header.tickOffsetHi = static_cast<uint32_t>(config.tickOffset >> 32u);
  header.halfMax =
      0.5f * static_cast<float>((std::max)(config.width, config.height));
  header.halfWidth = 0.5f * static_cast<float>(config.width);
  header.halfHeight = 0.5f * static_cast<float>(config.height);
  header.reserved1 = 0.0f;
  std::memcpy(buffer.data(), &header, sizeof(header));

  std::vector<float> packed(static_cast<size_t>(vec4Count) * 4u, 0.0f);

  const auto paramsFor = [&](bool isCamera) -> const TransformParams& {
    if (isCamera) {
      if (!cameraSystem.camera) {
        throw std::invalid_argument(
            "UniformLayout expects camera transform params");
      }
      return cameraSystem.camera->params;
    }
    return cameraSystem.system.params;
  };

  for (const auto& slot : layout.affineSlots) {
    const auto& params = paramsFor(slot.isCamera);
    auto it = params.find(slot.index);
    if (it == params.end() || it->second.size() < 6u) {
      throw MissingParameterError(it == params.end() ? 0 : it->second.size());
    }
    for (size_t i = 0; i < 6u; ++i) {
      packed[slot.floatOffset + i] = it->second[i];
    }
  }

  for (const auto& slot : layout.paramSlots) {
    const auto& params = paramsFor(slot.isCamera);
    auto it = params.find(slot.index);
    if (it == params.end() || slot.paramIndex >= it->second.size()) {
      throw MissingParameterError(slot.paramIndex);
    }
    packed[slot.floatOffset] = it->second[slot.paramIndex];
  }

  size_t rcCursor = 0;
  size_t wsCursor = 0;
  WeightCollector sysWeights(false, layout, rcCursor, wsCursor, packed);
  sysWeights.collectTransform(cameraSystem.system.transform, TransformIndex());
  if (cameraSystem.camera) {
    WeightCollector camWeights(true, layout, rcCursor, wsCursor, packed);
    camWeights.collectTransform(cameraSystem.camera->transform,
                                TransformIndex());
  }
  if (rcCursor != layout.randomChoiceSlots.size() ||
      wsCursor != layout.weightedSumSlots.size()) {
    throw std::invalid_argument("UniformLayout weight slot count mismatch");
  }

  std::memcpy(buffer.data() + kHeaderByteSize, packed.data(),
              packed.size() * sizeof(float));
  return buffer;
}

}  // namespace chaoskit::core
