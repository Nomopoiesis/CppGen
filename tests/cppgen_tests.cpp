#include "compiler_config.hpp"
#include "test_utils.hpp"
#include <gtest/gtest.h>

#define CPPGEN_IMPLEMENTATION
#include "cppgen/cppgen.hpp"

TEST(CompileRunPipeline, TrivialProgramCompilesAndRuns) {
  std::string code = R"(
        int main() { return 42; }
    )";
  auto result = CompileAndRun(code, CPPGEN_TEST_CXX);
  ASSERT_TRUE(result.success) << "Compilation failed: " << result.stderr_output;
  EXPECT_EQ(result.exit_code, 42);
}

TEST(CompileRunPipeline, ProgramWithStdoutOutput) {
  std::string code = R"(
        #include <iostream>
        int main() {
            std::cout << "Hello from generated code";
            return 0;
        }
    )";
  auto result = CompileAndRun(code, CPPGEN_TEST_CXX);
  ASSERT_TRUE(result.success) << "Compilation failed: " << result.stderr_output;
  EXPECT_EQ(result.exit_code, 0);
  EXPECT_TRUE(result.stdout_output.find("Hello from generated code") !=
              std::string::npos)
      << "Expected 'Hello from generated code' in stdout, got: "
      << result.stdout_output;
}

TEST(BasicCodeGeneration, SimpleCode) {
  cppgen::CodeUnit code;
  code.Add<cppgen::RawText>("int main() { return 42; }");
  EXPECT_EQ(code.EmitCode(), "int main() { return 42; }\n");
}

TEST(BasicCodeGeneration, SimpleNamespace) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  code.Add<cppgen::Namespace>("MyNamespace");
  auto result_str =
      // clang-format off
R"(
namespace MyNamespace {
} // namespace MyNamespace
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, SimpleStruct) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  code.Add<cppgen::Struct>("MyStruct");
  auto result_str =
      // clang-format off
R"(
struct MyStruct {
}; // struct MyStruct
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, SimpleStructWithMembers) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  auto &struct_ = code.Add<cppgen::Struct>("MyStruct");
  struct_.Add<cppgen::Variable>("int", "my_int");
  struct_.Add<cppgen::Variable>("int", "my_int2").SetInitializer("10");
  struct_.Add<cppgen::Variable>("float", "my_float").SetInitializer("3.14");
  struct_.Add<cppgen::Variable>("double", "my_double").SetInitializer("2.71");
  struct_.Add<cppgen::Variable>("char", "my_char").SetInitializer("'a'");
  struct_.Add<cppgen::Variable>("bool", "my_bool").SetInitializer("true");
  struct_.Add<cppgen::Variable>("string", "my_string")
      .SetInitializer("\"Hello, World!\"");
  struct_.Add<cppgen::Variable>("vector<int>", "my_vector")
      .SetInitializer("std::vector<int>{1, 2, 3}");
  struct_.Add<cppgen::Variable>("map<int, string>", "my_map")
      .SetInitializer("std::map<int, string>{{1, \"Hello\"}, {2, \"World\"}}");
  struct_.Add<cppgen::Variable>("set<int>", "my_set")
      .SetInitializer("std::set<int>{1, 2, 3}");
  struct_.Add<cppgen::Variable>("int*", "my_int_ptr").SetInitializer("&my_int");
  struct_.Add<cppgen::Variable>("int**", "my_int_ptr_ptr")
      .SetInitializer("&my_int_ptr");
  struct_.Add<cppgen::Variable>("int***", "my_int_ptr_ptr_ptr")
      .SetInitializer("&my_int_ptr_ptr");
  struct_.Add<cppgen::Variable>("int****", "my_int_ptr_ptr_ptr_ptr")
      .SetInitializer("&my_int_ptr_ptr_ptr");
  struct_.Add<cppgen::Variable>("int*****", "my_int_ptr_ptr_ptr_ptr_ptr")
      .SetInitializer("&my_int_ptr_ptr_ptr_ptr");
  struct_.Add<cppgen::Variable>("int******", "my_int_ptr_ptr_ptr_ptr_ptr_ptr")
      .SetInitializer("&my_int_ptr_ptr_ptr_ptr_ptr");
  struct_
      .Add<cppgen::Variable>("int*******", "my_int_ptr_ptr_ptr_ptr_ptr_ptr_ptr")
      .SetInitializer("&my_int_ptr_ptr_ptr_ptr_ptr_ptr");
  struct_
      .Add<cppgen::Variable>("int********",
                             "my_int_ptr_ptr_ptr_ptr_ptr_ptr_ptr_ptr")
      .SetInitializer("&my_int_ptr_ptr_ptr_ptr_ptr_ptr_ptr");

  auto result_str =
      // clang-format off
R"(
struct MyStruct {
  int my_int;
  int my_int2 = 10;
  float my_float = 3.14;
  double my_double = 2.71;
  char my_char = 'a';
  bool my_bool = true;
  string my_string = "Hello, World!";
  vector<int> my_vector = std::vector<int>{1, 2, 3};
  map<int, string> my_map = std::map<int, string>{{1, "Hello"}, {2, "World"}};
  set<int> my_set = std::set<int>{1, 2, 3};
  int* my_int_ptr = &my_int;
  int** my_int_ptr_ptr = &my_int_ptr;
  int*** my_int_ptr_ptr_ptr = &my_int_ptr_ptr;
  int**** my_int_ptr_ptr_ptr_ptr = &my_int_ptr_ptr_ptr;
  int***** my_int_ptr_ptr_ptr_ptr_ptr = &my_int_ptr_ptr_ptr_ptr;
  int****** my_int_ptr_ptr_ptr_ptr_ptr_ptr = &my_int_ptr_ptr_ptr_ptr_ptr;
  int******* my_int_ptr_ptr_ptr_ptr_ptr_ptr_ptr = &my_int_ptr_ptr_ptr_ptr_ptr_ptr;
  int******** my_int_ptr_ptr_ptr_ptr_ptr_ptr_ptr_ptr = &my_int_ptr_ptr_ptr_ptr_ptr_ptr_ptr;
}; // struct MyStruct
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ArrayVariableWithoutInitializer) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  code.Add<cppgen::ArrayVariable>("int", "arr");
  auto result_str =
      // clang-format off
R"(
int arr[];
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ArrayVariableWithSize) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  code.Add<cppgen::ArrayVariable>("int", "arr", "10");
  auto result_str =
      // clang-format off
R"(
int arr[10];
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ArrayVariableWithSizeSetSize) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  code.Add<cppgen::ArrayVariable>("int", "buf").SetSize("N");
  auto result_str =
      // clang-format off
R"(
int buf[N];
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ArrayVariableWithInitializerList) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  cppgen::InitializerList list;
  list.AddValue("1").AddValue("2").AddValue("3");
  code.Add<cppgen::ArrayVariable>("int", "arr").SetInitializer(std::move(list));
  auto result_str =
      // clang-format off
R"(
int arr[] = {
  1, 2, 3
};
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ArrayVariableWithCompactInitializerList) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  cppgen::InitializerList list;
  list.AddValue("1").AddValue("2").AddValue("3").SetCompact(true);
  code.Add<cppgen::ArrayVariable>("int", "arr").SetInitializer(std::move(list));
  auto result_str =
      // clang-format off
R"(
int arr[] = {1, 2, 3};
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ArrayVariableWithCompactNamedInitializerList) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  cppgen::InitializerList list;
  list.AddValue("x", "1").AddValue("y", "2").SetCompact(true);
  code.Add<cppgen::ArrayVariable>("Point", "pt")
      .SetInitializer(std::move(list));
  auto result_str =
      // clang-format off
R"(
Point pt[] = {.x = 1, .y = 2};
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ArrayVariableWithInitializerListWrapsWhenLong) {
  cppgen::CodeUnit code;
  code.SetMaxLineLength(10);
  code.Add<cppgen::NewLine>();
  cppgen::InitializerList list;
  list.AddValue("1").AddValue("2").AddValue("3").AddValue("4").AddValue("5");
  code.Add<cppgen::ArrayVariable>("int", "arr").SetInitializer(std::move(list));
  auto result_str =
      // clang-format off
R"(
int arr[] = {
  1, 2, 
  3, 4, 5
};
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ArrayVariableWithNamedInitializerList) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  cppgen::InitializerList list;
  list.AddValue("x", "1").AddValue("y", "2").AddValue("z", "3");
  code.Add<cppgen::ArrayVariable>("Point", "pt")
      .SetInitializer(std::move(list));
  auto result_str =
      // clang-format off
R"(
Point pt[] = {
  .x = 1, .y = 2, .z = 3
};
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ArrayVariableWithMixedInitializerList) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  cppgen::InitializerList list;
  list.AddValue("0").AddValue("width", "100").AddValue("height", "50");
  code.Add<cppgen::ArrayVariable>("Config", "cfg")
      .SetInitializer(std::move(list));
  auto result_str =
      // clang-format off
R"(
Config cfg[] = {
  0, .width = 100, .height = 50
};
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ArrayVariableWithStringInitializer) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  code.Add<cppgen::ArrayVariable>("int", "arr").SetInitializer("{1, 2, 3}");
  auto result_str =
      // clang-format off
R"(
int arr[] = {
  {1, 2, 3}
};
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ArrayVariableWithNamedInitializerListWrapsWhenLong) {
  cppgen::CodeUnit code;
  code.SetMaxLineLength(22);
  code.Add<cppgen::NewLine>();
  cppgen::InitializerList list;
  list.AddValue("x", "1").AddValue("y", "2").AddValue("z", "3");
  code.Add<cppgen::ArrayVariable>("Point", "pt")
      .SetInitializer(std::move(list));
  auto result_str =
      // clang-format off
R"(
Point pt[] = {
  .x = 1, .y = 2, 
  .z = 3
};
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, VariableWithSpecifiers) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  code.Add<cppgen::Variable>("int", "x")
      .AddSpecifier("static")
      .AddSpecifier("constexpr")
      .SetInitializer("0");
  auto result_str =
      // clang-format off
R"(
static constexpr int x = 0;
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ArrayVariableWithSpecifiers) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  cppgen::InitializerList list;
  list.AddValue("1").AddValue("2").AddValue("3");
  code.Add<cppgen::ArrayVariable>("int", "arr")
      .AddSpecifier("constexpr")
      .SetInitializer(std::move(list));
  auto result_str =
      // clang-format off
R"(
constexpr int arr[] = {
  1, 2, 3
};
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, FunctionWithSpecifiers) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  auto &function = code.Add<cppgen::Function>("void", "f");
  function.AddSpecifier("inline").AddSpecifier("static");
  function.Add<cppgen::RawText>("return;");
  auto result_str =
      // clang-format off
R"(
inline static void f()
{
  return;
}
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ArrayOfStructWithMixedInitializers) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  auto &struct_point = code.Add<cppgen::Struct>("Point");
  struct_point.Add<cppgen::Variable>("int", "x");
  struct_point.Add<cppgen::Variable>("int", "y");
  code.Add<cppgen::NewLine>();
  cppgen::InitializerList second_elem;
  second_elem.AddValue("x", "10").AddValue("y", "20").SetCompact(true);
  cppgen::InitializerList third_elem;
  third_elem.AddValue("15").AddValue("25").SetCompact(true);
  cppgen::InitializerList list;
  list.AddValue("{ 1, 2 }"); // first element: positional struct init
  list.AddValue(std::move(second_elem)); // second: named in nested list
  list.AddValue(std::move(third_elem));  // third: positional in nested list
  code.Add<cppgen::ArrayVariable>("Point", "pts")
      .SetInitializer(std::move(list));
  auto result_str =
      // clang-format off
R"(
struct Point {
  int x;
  int y;
}; // struct Point

Point pts[] = {
  { 1, 2 }, {.x = 10, .y = 20}, {15, 25}
};
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, SimpleFunction) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  auto &function = code.Add<cppgen::Function>("int", "my_function");
  function.AddParameter("int", "my_int");
  function.AddParameter("int", "my_int2");
  function.AddParameter("float", "my_float");
  function.AddParameter("double", "my_double");
  function.AddParameter("char", "my_char");
  function.AddParameter("bool", "my_bool").SetDefaultValue("true");
  function.AddParameter("string", "my_string")
      .SetDefaultValue("\"Hello, World!\"");
  function.Add<cppgen::RawText>("return my_int + my_int2 + my_float + "
                                "my_double + my_char + my_bool + my_string;");
  auto result_str =
      // clang-format off
R"(
int my_function(int my_int, int my_int2, float my_float, double my_double, char my_char, bool my_bool = true, string my_string = "Hello, World!")
{
  return my_int + my_int2 + my_float + my_double + my_char + my_bool + my_string;
}
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, IncludeWithQuotes) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  code.Add<cppgen::Include>("my_header.hpp", true);
  auto result_str =
      // clang-format off
R"(
#include "my_header.hpp"
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, SimpleCodeWithInclude) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  code.Add<cppgen::Include>("iostream");
  code.Add<cppgen::Include>("string");
  code.Add<cppgen::NewLine>();
  auto &namespace_ = code.Add<cppgen::Namespace>("CppGenTestNS");
  auto &struct_ = namespace_.Add<cppgen::Struct>("CppGenTestStruct");
  struct_.Add<cppgen::Variable>("int", "my_int").SetInitializer("10");
  struct_.Add<cppgen::Variable>("int", "my_int2").SetInitializer("10");
  struct_.Add<cppgen::Variable>("float", "my_float").SetInitializer("3.14");
  struct_.Add<cppgen::Variable>("double", "my_double").SetInitializer("2.71");
  struct_.Add<cppgen::Variable>("char", "my_char").SetInitializer("'a'");
  struct_.Add<cppgen::Variable>("bool", "my_bool").SetInitializer("true");
  struct_.Add<cppgen::Variable>("std::string", "my_string")
      .SetInitializer("\"Hello, World!\"");
  namespace_.Add<cppgen::NewLine>();
  auto &function = namespace_.Add<cppgen::Function>("int", "f");
  function.Add<cppgen::Variable>("CppGenTestStruct", "my_struct");
  function.Add<cppgen::RawText>("my_struct.my_int = 42;");
  function.Add<cppgen::RawText>("return my_struct.my_int;");
  code.Add<cppgen::NewLine>();
  auto &main_function = code.Add<cppgen::Function>("int", "main");
  main_function.AddParameter("int", "argc");
  main_function.AddParameter("char**", "argv");
  main_function.Add<cppgen::RawText>("return CppGenTestNS::f();");
  auto result_str =
      // clang-format off
R"(
#include <iostream>
#include <string>

namespace CppGenTestNS {
  struct CppGenTestStruct {
    int my_int = 10;
    int my_int2 = 10;
    float my_float = 3.14;
    double my_double = 2.71;
    char my_char = 'a';
    bool my_bool = true;
    std::string my_string = "Hello, World!";
  }; // struct CppGenTestStruct

  int f()
  {
    CppGenTestStruct my_struct;
    my_struct.my_int = 42;
    return my_struct.my_int;
  }
} // namespace CppGenTestNS

int main(int argc, char** argv)
{
  return CppGenTestNS::f();
}
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
  auto result = CompileAndRun(code.EmitCode(), CPPGEN_TEST_CXX);
  ASSERT_TRUE(result.success) << "Compilation failed: " << result.stderr_output;
  EXPECT_EQ(result.exit_code, 42);
}

TEST(BasicCodeGeneration, StructWithDefaultAddPreservesBackwardCompat) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  auto &s = code.Add<cppgen::Struct>("Point");
  s.Add<cppgen::Variable>("int", "x");
  s.Add<cppgen::Variable>("int", "y");
  auto result_str =
      // clang-format off
R"(
struct Point {
  int x;
  int y;
}; // struct Point
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, StructWithAccessSections) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  auto &s = code.Add<cppgen::Struct>("Node");
  s.AddPublic().Add<cppgen::Variable>("int", "value");
  s.AddPrivate().Add<cppgen::Variable>("int", "m_internal");
  auto result_str =
      // clang-format off
R"(
struct Node {
public:
  int value;
private:
  int m_internal;
}; // struct Node
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ClassWithAccessSections) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  auto &cls = code.Add<cppgen::Class>("Renderer");
  cls.AddPublic().Add<cppgen::Function>("void", "Draw");
  auto &priv = cls.AddPrivate();
  priv.Add<cppgen::Variable>("int", "m_width");
  priv.Add<cppgen::Variable>("int", "m_height");
  auto result_str =
      // clang-format off
R"(
class Renderer {
public:
  void Draw()
  {
  }
private:
  int m_width;
  int m_height;
}; // class Renderer
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}

TEST(BasicCodeGeneration, ClassDefaultAndNamedSectionsMix) {
  cppgen::CodeUnit code;
  code.Add<cppgen::NewLine>();
  auto &cls = code.Add<cppgen::Class>("Timer");
  // default Add<> (no label) followed by named sections
  cls.Add<cppgen::Variable>("static int", "s_count");
  cls.AddPublic().Add<cppgen::Function>("void", "Start");
  cls.AddPrivate().Add<cppgen::Variable>("int", "m_ms");
  auto result_str =
      // clang-format off
R"(
class Timer {
  static int s_count;
public:
  void Start()
  {
  }
private:
  int m_ms;
}; // class Timer
)";
  // clang-format on
  EXPECT_EQ(code.EmitCode(), result_str);
}
