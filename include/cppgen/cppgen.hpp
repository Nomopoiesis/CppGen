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
#include <variant>
#include <vector>

namespace cppgen {

// --- CodeWriter ---
class CodeWriter {
public:
  virtual ~CodeWriter() = default;

  auto SetMaxLineLength(int max_length) -> void {
    m_max_line_length = max_length;
  }
  auto GetMaxLineLength() const -> int { return m_max_line_length; }
  auto GetCurrentLineLength() const -> int { return m_current_line_length; }

  template <bool indent = true>
  auto Write(std::string_view text) -> void {
    if constexpr (indent) {
      if (m_current_line_length == 0) {
        for (int i = 0; i < m_indent_level; ++i) {
          m_output << "  ";
          m_current_line_length += 2;
        }
      }
    }
    m_output << text;
    auto last_nl = text.rfind('\n');
    if (last_nl == std::string_view::npos) {
      m_current_line_length += static_cast<int>(text.size());
    } else {
      m_current_line_length = static_cast<int>(text.size() - (last_nl + 1));
    }
  }

  auto NewLine() -> void {
    m_output << "\n";
    m_current_line_length = 0;
  }

  auto WriteLine(std::string_view text) -> void {
    Write(text);
    NewLine();
  }

  /** If adding \a extra_chars would exceed max line length, emit newline and
   * indent so the next Write starts on a fresh line. No-op if max line length
   * is 0 (unlimited). */
  auto MaybeWrap(int extra_chars) -> void {
    if (m_max_line_length <= 0)
      return;
    if (m_current_line_length + extra_chars > m_max_line_length &&
        m_current_line_length > 0) {
      NewLine();
      if (m_indent_level > 0) {
        for (int i = 0; i < m_indent_level; ++i) {
          m_output << "  ";
          m_current_line_length += 2;
        }
      }
    }
  }

  auto IdentIn() -> void { ++m_indent_level; }
  auto IdentOut() -> void { --m_indent_level; }

  auto GetOutput() const -> std::string { return m_output.str(); }

  auto Clear() -> void {
    m_output.str("");
    m_output.clear();
    m_indent_level = 0;
    m_current_line_length = 0;
  }

private:
  int m_indent_level = 0;
  int m_current_line_length = 0;
  int m_max_line_length = 80;
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

  auto SetMaxLineLength(int max_length) -> void {
    m_writer.SetMaxLineLength(max_length);
  }
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
  Include(const std::string &include, bool use_quotes = false)
      : m_include(include), m_use_quotes(use_quotes) {}
  virtual ~Include() = default;
  auto Emit(CodeWriter &writer) -> std::string override;

private:
  std::string m_include;
  bool m_use_quotes = false;
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

// --- Initializer list ---
using InitializerListValue = std::pair<std::optional<std::string>, std::string>;

class InitializerList : public CodeElement {
public:
  InitializerList() = default;
  virtual ~InitializerList() = default;
  auto Emit(CodeWriter &writer) -> std::string override;

  auto AddValue(const std::string &value) -> InitializerList &;
  auto AddValue(const std::string &name, const std::string &value)
      -> InitializerList &;
  auto AddValue(InitializerList list) -> InitializerList &;

  auto SetCompact(bool compact = true) -> InitializerList & {
    m_compact = compact;
    return *this;
  }

private:
  std::vector<std::variant<InitializerListValue, InitializerList>> m_entries;
  bool m_compact = false;
};

// --- Variable ---
class Variable : public CodeElement {
public:
  Variable(const std::string &type, const std::string &name)
      : m_type(type), m_name(name) {}
  virtual ~Variable() = default;
  auto Emit(CodeWriter &writer) -> std::string override;

  auto AddSpecifier(const std::string &specifier) -> Variable & {
    m_specifiers.push_back(specifier);
    return *this;
  }
  auto SetInitializer(const std::string &initializer) -> Variable & {
    m_initializer = initializer;
    return *this;
  }

protected:
  std::string m_type;
  std::string m_name;
  std::vector<std::string> m_specifiers;

private:
  std::optional<std::string> m_initializer;
};

// --- ArrayVariable (derived from Variable) ---
class ArrayVariable : public Variable {
public:
  ArrayVariable(const std::string &type, const std::string &name,
                std::optional<std::string> size = std::nullopt)
      : Variable(type, name), m_array_size(std::move(size)) {}
  virtual ~ArrayVariable() = default;
  auto Emit(CodeWriter &writer) -> std::string override;

  auto AddSpecifier(const std::string &specifier) -> ArrayVariable & {
    Variable::AddSpecifier(specifier);
    return *this;
  }
  auto SetSize(std::string size) -> ArrayVariable & {
    m_array_size = std::move(size);
    return *this;
  }
  auto SetInitializer(const std::string &initializer) -> ArrayVariable &;
  auto SetInitializer(InitializerList list) -> ArrayVariable &;

private:
  std::optional<std::string> m_array_size;
  std::optional<InitializerList> m_array_initializer;
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

  auto AddSpecifier(const std::string &specifier) -> Function & {
    m_specifiers.push_back(specifier);
    return *this;
  }
  auto AddParameter(const std::string &type, const std::string &name)
      -> ParameterDeclaration & {
    m_parameters.emplace_back(type, name);
    return m_parameters.back();
  }

private:
  std::string m_name;
  std::string m_return_type;
  std::vector<std::string> m_specifiers;
  std::vector<ParameterDeclaration> m_parameters;
};

// --- EnumDecl ---
// Scoped=true  → "enum class Name : Type { ... };"
// Scoped=false → "enum Name : Type { ... };"
// Emit is defined inline because template implementations must be
// header-visible.
template <bool Scoped = true>
class EnumDecl : public CodeElement {
public:
  EnumDecl(const std::string &name) : m_name(name) {}
  EnumDecl(const std::string &name, const std::string &underlying_type)
      : m_name(name), m_underlying_type(underlying_type) {}
  virtual ~EnumDecl() = default;

  auto AddValue(const std::string &name) -> EnumDecl & {
    m_values.emplace_back(name, std::nullopt);
    return *this;
  }
  auto AddValue(const std::string &name, const std::string &value)
      -> EnumDecl & {
    m_values.emplace_back(name, std::optional<std::string>{value});
    return *this;
  }

  auto Emit(CodeWriter &writer) -> std::string override {
    std::string decl = Scoped ? "enum class " : "enum ";
    decl += m_name;
    if (m_underlying_type)
      decl += " : " + *m_underlying_type;
    decl += " {";
    writer.WriteLine(decl);
    writer.IdentIn();
    for (size_t i = 0; i < m_values.size(); ++i) {
      const auto &[vname, vval] = m_values[i];
      std::string line = vname;
      if (vval)
        line += " = " + *vval;
      if (i < m_values.size() - 1)
        line += ",";
      writer.WriteLine(line);
    }
    writer.IdentOut();
    writer.WriteLine("}; // " + std::string(Scoped ? "enum class " : "enum ") +
                     m_name);
    return m_name;
  }

private:
  std::string m_name;
  std::optional<std::string> m_underlying_type;
  std::vector<std::pair<std::string, std::optional<std::string>>> m_values;
};

using Enum = EnumDecl<false>;
using EnumClass = EnumDecl<true>;

// --- Access specifier ---
enum class Access { Public, Private, Protected };

// --- Class member types ---
// All types valid as direct members of a struct/class.
// Extend by adding new variant alternatives and is_class_member
// specialisations.
using MemberVariant =
    std::variant<std::unique_ptr<Variable>, std::unique_ptr<ArrayVariable>,
                 std::unique_ptr<Function>, std::unique_ptr<EnumDecl<true>>,
                 std::unique_ptr<EnumDecl<false>>>;

template <typename T>
inline constexpr bool is_class_member_v =
    std::disjunction_v<std::is_same<T, Variable>,
                       std::is_same<T, ArrayVariable>,
                       std::is_same<T, Function>,
                       std::is_same<T, EnumDecl<true>>,
                       std::is_same<T, EnumDecl<false>>>;

// --- MemberSection ---
// A typed, optionally-labelled group of class/struct members.
class MemberSection {
public:
  MemberSection() = default;
  MemberSection(MemberSection &&) = default;
  MemberSection &operator=(MemberSection &&) = default;
  MemberSection(const MemberSection &) = delete;
  MemberSection &operator=(const MemberSection &) = delete;

  template <typename T, typename... Args>
  auto Add(Args &&...args) -> T & {
    static_assert(is_class_member_v<T>, "T is not a valid class/struct member");
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    T &ref = *ptr;
    m_members.emplace_back(std::move(ptr));
    return ref;
  }

  auto IsEmpty() const -> bool { return m_members.empty(); }
  auto Emit(CodeWriter &writer) -> void;

private:
  std::vector<MemberVariant> m_members;
};

// --- AggregateType (base for Struct and Class) ---
class AggregateType : public CodeElement {
public:
  AggregateType(const std::string &name) : m_name(name) {}
  AggregateType(const AggregateType &) = delete;
  AggregateType &operator=(const AggregateType &) = delete;
  virtual ~AggregateType() = default;

  // Convenience Add<T>() — goes into the implicit default section (no label).
  // Preserves backward-compatible usage of Struct.
  template <typename T, typename... Args>
  auto Add(Args &&...args) -> T & {
    return m_default_section.Add<T>(std::forward<Args>(args)...);
  }

  auto AddPublic() -> MemberSection & { return AddSection(Access::Public); }
  auto AddPrivate() -> MemberSection & { return AddSection(Access::Private); }
  auto AddProtected() -> MemberSection & {
    return AddSection(Access::Protected);
  }

  auto Emit(CodeWriter &writer) -> std::string override;

protected:
  virtual auto GetKeyword() const -> std::string_view = 0;

private:
  auto AddSection(Access access) -> MemberSection & {
    m_sections.emplace_back(access, std::make_unique<MemberSection>());
    return *m_sections.back().second;
  }

  std::string m_name;
  MemberSection m_default_section;
  std::vector<std::pair<Access, std::unique_ptr<MemberSection>>> m_sections;
};

// --- Struct ---
class Struct : public AggregateType {
public:
  Struct(const std::string &name) : AggregateType(name) {}
  virtual ~Struct() = default;

protected:
  auto GetKeyword() const -> std::string_view override { return "struct"; }
};

// --- Class ---
class Class : public AggregateType {
public:
  Class(const std::string &name) : AggregateType(name) {}
  virtual ~Class() = default;

protected:
  auto GetKeyword() const -> std::string_view override { return "class"; }
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
  if (m_use_quotes)
    writer.WriteLine("#include \"" + m_include + "\"");
  else
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

inline auto InitializerList::Emit(CodeWriter &writer) -> std::string {
  if (m_compact) {
    writer.Write<false>("{");
  } else {
    writer.Write<false>("{\n");
    writer.IdentIn();
  }
  for (size_t i = 0; i < m_entries.size(); ++i) {
    const bool is_last = (i == m_entries.size() - 1);
    std::visit(
        [&writer, is_last](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, InitializerListValue>) {
            std::string chunk =
                arg.first ? "." + *arg.first + " = " + arg.second : arg.second;
            if (!is_last)
              chunk += ", ";
            int need = static_cast<int>(chunk.size());
            writer.MaybeWrap(need);
            if (arg.first) {
              writer.Write(".");
              writer.Write(*arg.first + " = ");
            }
            writer.Write(arg.second);
            if (!is_last)
              writer.Write(", ");
          } else if constexpr (std::is_same_v<T, InitializerList>) {
            int need = 2;
            if (!is_last)
              need += 2;
            writer.MaybeWrap(need);
            arg.Emit(writer);
            if (!is_last)
              writer.Write<false>(", ");
          }
        },
        m_entries[i]);
  }
  if (m_compact) {
    writer.Write<false>("}");
  } else {
    writer.IdentOut();
    writer.Write<false>("\n}");
  }
  return "";
}

inline auto InitializerList::AddValue(const std::string &value)
    -> InitializerList & {
  m_entries.emplace_back(InitializerListValue{std::nullopt, value});
  return *this;
}

inline auto InitializerList::AddValue(const std::string &name,
                                      const std::string &value)
    -> InitializerList & {
  m_entries.emplace_back(InitializerListValue{std::optional{name}, value});
  return *this;
}

inline auto InitializerList::AddValue(InitializerList list)
    -> InitializerList & {
  m_entries.emplace_back(std::move(list));
  return *this;
}

inline auto Variable::Emit(CodeWriter &writer) -> std::string {
  std::stringstream ss;
  for (const auto &s : m_specifiers) {
    ss << s << " ";
  }
  ss << m_type << " " << m_name;
  if (m_initializer) {
    ss << " = " << *m_initializer;
  }
  ss << ";";
  writer.WriteLine(ss.str());
  return m_name;
}

inline auto ArrayVariable::SetInitializer(const std::string &initializer)
    -> ArrayVariable & {
  InitializerList list;
  list.AddValue(initializer);
  m_array_initializer = std::move(list);
  return *this;
}

inline auto ArrayVariable::SetInitializer(InitializerList list)
    -> ArrayVariable & {
  m_array_initializer = std::move(list);
  return *this;
}

inline auto ArrayVariable::Emit(CodeWriter &writer) -> std::string {
  std::string decl;
  for (const auto &s : m_specifiers)
    decl += s + " ";
  decl += m_type + " " + m_name + "[";
  if (m_array_size)
    decl += *m_array_size;
  decl += "]";
  writer.Write(decl);
  if (m_array_initializer) {
    writer.Write<false>(" = ");
    m_array_initializer->Emit(writer);
  }
  writer.WriteLine(";");
  return m_name;
}

inline auto MemberSection::Emit(CodeWriter &writer) -> void {
  for (auto &member : m_members) {
    std::visit([&writer](auto &ptr) { ptr->Emit(writer); }, member);
  }
}

inline auto AggregateType::Emit(CodeWriter &writer) -> std::string {
  std::string kw(GetKeyword());
  writer.WriteLine(kw + " " + m_name + " {");
  writer.IdentIn();
  m_default_section.Emit(writer);
  for (auto &[access, section] : m_sections) {
    writer.IdentOut();
    switch (access) {
      case Access::Public:
        writer.WriteLine("public:");
        break;
      case Access::Private:
        writer.WriteLine("private:");
        break;
      case Access::Protected:
        writer.WriteLine("protected:");
        break;
    }
    writer.IdentIn();
    section->Emit(writer);
  }
  writer.IdentOut();
  writer.WriteLine("}; // " + kw + " " + m_name);
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
  for (const auto &s : m_specifiers) {
    writer.Write<false>(s + " ");
  }
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
