#pragma once

#include <cstdlib>
#include <cctype>
#include <fstream>
#include <system_error>
#include <filesystem>
#include <iterator>
#include <string>

#ifndef _WIN32
#include <sys/wait.h>
#endif

namespace fs = std::filesystem;

struct CompileRunResult {
    bool success = false;
    int exit_code = -1;
    std::string stdout_output;
    std::string stderr_output;
};

inline std::string quote_path(const std::string& path) {
    return "\"" + path + "\"";
}

#ifdef _WIN32
inline std::string get_vcvarsall_path(const std::string& compiler) {
    std::string path = compiler;
    for (auto& c : path) if (c == '\\') c = '/';
    std::string::size_type pos = path.find("/VC/Tools/");
    if (pos == std::string::npos) return {};
    return path.substr(0, pos + 4) + "Auxiliary/Build/vcvarsall.bat";
}

inline int run_via_batch(const std::string& cmd, const fs::path& bat_dir,
                        const std::string& setup_cmd = "") {
    fs::path bat_file = bat_dir;
    bat_file += "_run.bat";
    std::ofstream bat(bat_file);
    bat << "@echo off\n";
    if (!setup_cmd.empty()) {
        bat << setup_cmd << "\n";
        bat << "if errorlevel 1 exit /b 1\n";
    }
    bat << cmd << "\n";
    bat.close();
    int ret = std::system(bat_file.string().c_str());
    std::error_code ec;
    fs::remove(bat_file, ec);
    return ret;
}
#endif

inline void remove_ignore_error(const fs::path& p) {
    std::error_code ec;
    fs::remove(p, ec);
}

inline bool is_msvc(const std::string& compiler) {
    std::string lower = compiler;
    for (auto& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return lower.find("cl.exe") != std::string::npos;
}

inline CompileRunResult CompileAndRun(const std::string& cpp_source,
                                     const std::string& compiler) {
    CompileRunResult result;
    if (compiler.empty()) {
        result.stderr_output = "CPPGEN_TEST_CXX not set - compiler path missing";
        return result;
    }

    fs::path tmp_dir = fs::temp_directory_path();
    fs::path base = tmp_dir / "cppgen_test";
    fs::path src_file = base;
    src_file += ".cpp";
    fs::path exe_file = base;
#ifdef _WIN32
    exe_file += ".exe";
#endif
    fs::path stdout_file = base;
    stdout_file += "_stdout.txt";
    fs::path stderr_file = base;
    stderr_file += "_stderr.txt";
    fs::path compile_err_file = base;
    compile_err_file += "_compile_err.txt";
    fs::path bat_base = tmp_dir / "cppgen_test";
    bat_base += "_";

    std::ofstream out(src_file);
    if (!out) {
        result.stderr_output = "Failed to write temp source file";
        return result;
    }
    out << cpp_source;
    out.close();
    if (!out) {
        result.stderr_output = "Failed to flush temp source file";
        return result;
    }

    std::string compile_cmd;
    std::string setup_cmd;
    if (is_msvc(compiler)) {
        std::string vcvarsall = get_vcvarsall_path(compiler);
        if (!vcvarsall.empty()) {
            setup_cmd = "call " + quote_path(vcvarsall) + " x64";
        }
        compile_cmd = quote_path(compiler) + " /EHsc /std:c++20 /Fe:" +
                      quote_path(exe_file.string()) + " " +
                      quote_path(src_file.string()) + " 2> " +
                      quote_path(compile_err_file.string());
    } else {
        compile_cmd = quote_path(compiler) + " -std=c++20 -o " +
                      quote_path(exe_file.string()) + " " +
                      quote_path(src_file.string()) + " 2> " +
                      quote_path(compile_err_file.string());
    }

#ifdef _WIN32
    int compile_ret = run_via_batch(compile_cmd, bat_base, setup_cmd);
#else
    int compile_ret = std::system(compile_cmd.c_str());
#endif
#ifdef _WIN32
    int compile_exit = compile_ret;
#else
    int compile_exit = WIFEXITED(compile_ret) ? WEXITSTATUS(compile_ret) : -1;
#endif
    if (compile_exit != 0) {
        result.stderr_output = "Compilation failed with exit code " + std::to_string(compile_exit);
        std::ifstream comp_err(compile_err_file);
        if (comp_err) {
            result.stderr_output += "\n";
            result.stderr_output += std::string(std::istreambuf_iterator<char>(comp_err), {});
        }
        remove_ignore_error(src_file);
        remove_ignore_error(compile_err_file);
        return result;
    }
    remove_ignore_error(compile_err_file);

    std::string run_cmd = quote_path(exe_file.string()) + " > " +
                         quote_path(stdout_file.string()) + " 2> " +
                         quote_path(stderr_file.string());
#ifdef _WIN32
    int run_ret = run_via_batch(run_cmd, bat_base);
#else
    int run_ret = std::system(run_cmd.c_str());
#endif
#ifdef _WIN32
    result.exit_code = run_ret;
#else
    result.exit_code = WIFEXITED(run_ret) ? WEXITSTATUS(run_ret) : -1;
#endif

    std::ifstream stdout_in(stdout_file);
    if (stdout_in) {
        result.stdout_output = std::string(std::istreambuf_iterator<char>(stdout_in), {});
    }
    std::ifstream stderr_in(stderr_file);
    if (stderr_in) {
        result.stderr_output = std::string(std::istreambuf_iterator<char>(stderr_in), {});
    }

    result.success = true;

    remove_ignore_error(src_file);
    remove_ignore_error(exe_file);
    remove_ignore_error(stdout_file);
    remove_ignore_error(stderr_file);

    return result;
}
