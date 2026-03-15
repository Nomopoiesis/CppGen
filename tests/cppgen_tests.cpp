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
