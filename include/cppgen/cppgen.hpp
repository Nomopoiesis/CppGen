#pragma once

/**
 * CppGen - C++ code generation library (header-only)
 *
 * Usage:
 *   1. Include this header where you need the API.
 *   2. In exactly one translation unit, define CPPGEN_IMPLEMENTATION before
 *      including:
 *
 *      #define CPPGEN_IMPLEMENTATION
 *      #include <cppgen/cppgen.hpp>
 *
 *   For header-only projects, define CPPGEN_IMPLEMENTATION in every file that
 *   includes the header (implementations are inline).
 */

#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace cppgen {

// --- CodeWriter ---
class CodeWriter {
public:
  virtual ~CodeWriter() = default;

  template <bool indent = true>
  auto Write(std::string_view text) -> void {
    if constexpr (indent) {
      for (int i = 0; i < m_indent_level; ++i) {
        m_output << "  ";
      }
    }
    m_output << text;
  }

  auto NewLine() -> void { m_output << "\n"; }

  auto WriteLine(std::string_view text) -> void {
    Write(text);
    NewLine();
  }

  auto IdentIn() -> void { ++m_indent_level; }
  auto IdentOut() -> void { --m_indent_level; }

  auto GetOutput() const -> std::string { return m_output.str(); }

  auto Clear() -> void {
    m_output.str("");
    m_output.clear();
    m_indent_level = 0;
  }

private:
  int m_indent_level = 0;
  std::stringstream m_output;
};

// --- CodeElement (base) ---
class CodeElement {
public:
  virtual ~CodeElement() = default;
  virtual auto Emit(CodeWriter &writer) -> std::string = 0;
};

// --- CodeBlock ---
class CodeBlock : public CodeElement {
public:
  CodeBlock() = default;
  CodeBlock(const CodeBlock &) = delete;
  CodeBlock &operator=(const CodeBlock &) = delete;
  virtual ~CodeBlock() = default;

  template <typename T, typename... Args>
  auto Add(Args &&...args) -> T & {
    elements_.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
    return static_cast<T &>(*elements_.back());
  }

protected:
  std::vector<std::unique_ptr<CodeElement>> elements_;
};

// --- CodeUnit ---
class CodeUnit : public CodeBlock {
public:
  CodeUnit() = default;
  virtual ~CodeUnit() = default;

  auto EmitCode() -> std::string { return Emit(m_writer); }

private:
  auto Emit(CodeWriter &writer) -> std::string override {
    m_writer.Clear();
    for (const auto &element : elements_) {
      element->Emit(m_writer);
    }
    return m_writer.GetOutput();
  }

private:
  CodeWriter m_writer;
};

// --- NewLine ---
class NewLine : public CodeElement {
public:
  NewLine() = default;
  virtual ~NewLine() = default;
  auto Emit(CodeWriter &writer) -> std::string override;
};

// --- RawText ---
class RawText : public CodeElement {
public:
  RawText(const std::string &text) : m_text(text) {}
  virtual ~RawText() = default;
  auto Emit(CodeWriter &writer) -> std::string override;

private:
  std::string m_text;
};

// --- Include ---
class Include : public CodeElement {
public:
  Include(const std::string &include) : m_include(include) {}
  virtual ~Include() = default;
  auto Emit(CodeWriter &writer) -> std::string override;

private:
  std::string m_include;
};

// --- Namespace ---
class Namespace : public CodeBlock {
public:
  Namespace(const std::string &name) : m_name(name) {}
  virtual ~Namespace() = default;
  auto Emit(CodeWriter &writer) -> std::string override;

private:
  std::string m_name;
};

// --- Variable ---
class Variable : public CodeElement {
public:
  Variable(const std::string &type, const std::string &name)
      : m_type(type), m_name(name) {}
  virtual ~Variable() = default;
  auto Emit(CodeWriter &writer) -> std::string override;

  auto SetInitializer(const std::string &initializer) -> Variable & {
    m_initializer = initializer;
    return *this;
  }

private:
  std::string m_type;
  std::string m_name;
  std::optional<std::string> m_initializer;
};

// --- Struct ---
class Struct : public CodeBlock {
public:
  Struct(const std::string &name) : m_name(name) {}
  virtual ~Struct() = default;
  auto Emit(CodeWriter &writer) -> std::string override;

private:
  std::string m_name;
};

// --- Function ---
class Function : public CodeBlock {
public:
  class ParameterDeclaration {
  public:
    ParameterDeclaration(const std::string &type, const std::string &name)
        : m_type(type), m_name(name) {}
    virtual ~ParameterDeclaration() = default;
    auto Emit(CodeWriter &writer) -> std::string;

    auto SetDefaultValue(const std::string &default_value)
        -> ParameterDeclaration & {
      m_default_value = default_value;
      return *this;
    }

  private:
    std::string m_type;
    std::string m_name;
    std::optional<std::string> m_default_value;
  };

  Function(const std::string &return_type, const std::string &name)
      : m_return_type(return_type), m_name(name) {}
  virtual ~Function() = default;
  auto Emit(CodeWriter &writer) -> std::string override;

  auto AddParameter(const std::string &type, const std::string &name)
      -> ParameterDeclaration & {
    m_parameters.emplace_back(type, name);
    return m_parameters.back();
  }

private:
  std::string m_name;
  std::string m_return_type;
  std::vector<ParameterDeclaration> m_parameters;
};

// =============================================================================
// Implementation (define CPPGEN_IMPLEMENTATION in one TU)
// =============================================================================

#ifdef CPPGEN_IMPLEMENTATION

inline auto NewLine::Emit(CodeWriter &writer) -> std::string {
  writer.NewLine();
  return "";
}

inline auto RawText::Emit(CodeWriter &writer) -> std::string {
  writer.WriteLine(m_text);
  return m_text;
}

inline auto Include::Emit(CodeWriter &writer) -> std::string {
  writer.WriteLine("#include <" + m_include + ">");
  return m_include;
}

inline auto Namespace::Emit(CodeWriter &writer) -> std::string {
  writer.WriteLine("namespace " + m_name + " {");
  writer.IdentIn();
  for (const auto &element : elements_) {
    element->Emit(writer);
  }
  writer.IdentOut();
  writer.WriteLine("} // namespace " + m_name);
  return m_name;
}

inline auto Variable::Emit(CodeWriter &writer) -> std::string {
  std::stringstream ss;
  ss << m_type << " " << m_name;
  if (m_initializer) {
    ss << " = " << *m_initializer;
  }
  ss << ";";
  writer.WriteLine(ss.str());
  return m_name;
}

inline auto Struct::Emit(CodeWriter &writer) -> std::string {
  writer.WriteLine("struct " + m_name + " {");
  writer.IdentIn();
  for (const auto &element : elements_) {
    element->Emit(writer);
  }
  writer.IdentOut();
  writer.WriteLine("}; // struct " + m_name);
  return m_name;
}

inline auto Function::ParameterDeclaration::Emit(CodeWriter &writer)
    -> std::string {
  writer.Write<false>(m_type + " " + m_name);
  if (m_default_value) {
    writer.Write<false>(" = " + *m_default_value);
  }
  return m_name;
}

inline auto Function::Emit(CodeWriter &writer) -> std::string {
  writer.Write(m_return_type + " " + m_name + "(");
  size_t parameter_count = m_parameters.size();
  for (size_t i = 0; i < parameter_count; ++i) {
    m_parameters[i].Emit(writer);
    if (i < parameter_count - 1) {
      writer.Write<false>(", ");
    }
  }
  writer.Write<false>(")\n");
  writer.WriteLine("{");
  writer.IdentIn();
  for (const auto &element : elements_) {
    element->Emit(writer);
  }
  writer.IdentOut();
  writer.WriteLine("}");
  return m_name;
}

#endif // CPPGEN_IMPLEMENTATION

} // namespace cppgen
