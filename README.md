# CppGen

A lightweight, header-only C++ code generation library. Build C++ source files programmatically using a simple tree-based API.

## Features

- Header-only — single include, no separate compilation required
- Composable elements: includes, namespaces, structs, functions, variables, raw text
- Automatic indentation tracking
- C++17

## Usage

In **exactly one** translation unit, define `CPPGEN_IMPLEMENTATION` before including:

```cpp
#define CPPGEN_IMPLEMENTATION
#include <cppgen/cppgen.hpp>
```

All other files include without the define.

### Example

```cpp
#define CPPGEN_IMPLEMENTATION
#include <cppgen/cppgen.hpp>

int main() {
    cppgen::CodeUnit code;
    code.Add<cppgen::Include>("iostream");
    code.Add<cppgen::NewLine>();

    auto& ns = code.Add<cppgen::Namespace>("MyApp");
    auto& fn = ns.Add<cppgen::Function>("int", "greet");
    fn.AddParameter("int", "x");
    fn.Add<cppgen::RawText>("return x + 1;");

    code.Add<cppgen::NewLine>();
    auto& main_fn = code.Add<cppgen::Function>("int", "main");
    main_fn.Add<cppgen::RawText>("return MyApp::greet(41);");

    std::cout << code.EmitCode();
}
```

## Building

Requires CMake 3.14+ and a C++17 compiler.

```sh
cmake -B build
cmake --build build
ctest --test-dir build
```

Tests use [GoogleTest](https://github.com/google/googletest) (fetched automatically via CMake FetchContent) and include a compile-and-run pipeline that invokes the system C++ compiler on generated code.

To skip building tests:

```sh
cmake -B build -DCPPGEN_BUILD_TESTS=OFF
```

## Elements

| Element     | Description                              |
|-------------|------------------------------------------|
| `CodeUnit`  | Root container; call `EmitCode()` to get the generated string |
| `Namespace` | `namespace Name { ... }`                 |
| `Struct`    | `struct Name { ... };`                   |
| `Function`  | Function definition with typed parameters and optional default values |
| `Variable`  | Typed variable declaration with optional initializer |
| `ArrayVariable` | Array variable; use `SetInitializer(InitializerList)` or `SetInitializer(string)` |
| `InitializerList` | Braced initializer list; supports positional values, `.name = value`, and nested lists |
| `Include`   | `#include <header>`                      |
| `RawText`   | Arbitrary text line                      |
| `NewLine`   | Blank line                               |

### Initializer lists

By default, `InitializerList` emits multi-line output with indentation:

```cpp
list.AddValue("1").AddValue("2").AddValue("3");
// → {
//     1, 2, 3
//   }
```

Call `SetCompact(true)` to emit the list on a single line with no newlines or indent:

```cpp
list.AddValue("1").AddValue("2").AddValue("3").SetCompact(true);
// → { 1, 2, 3 }
```

Nested lists use their own compact setting when emitted.
