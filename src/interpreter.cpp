#include "interpreter.h"

#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendDiagnostic.h>

#include <llvm/Support/DynamicLibrary.h>  // LoadLibraryPermanently
#include <llvm/Support/Error.h>           // ExitOnError
#include <llvm/Support/Signals.h>         // RunInterruptHandlers
#include <llvm/Support/TargetSelect.h>    // llvm::Initialize*

#include <fmt/core.h>
#include <string>

Interpreter::Interpreter(std::vector<const char*>& args) {
  llvm::ExitOnError check;
  auto compiler = check(clang::IncrementalCompilerBuilder::create(args));

  llvm::install_fatal_error_handler(
      error_handler, static_cast<void*>(&compiler->getDiagnostics()));
  compiler->LoadRequestedPlugins();
  interpreter = check(clang::Interpreter::create(std::move(compiler)));

  run(R"(
#include <string>
#include <sstream>

int main(){}

// HACK force the symbol to be materialized ahead of time
// TODO research why template instantiation doesn't work later
template class std::basic_string<char>;
template class std::basic_ostringstream<char>;
)");
}

Interpreter::~Interpreter() {
  llvm::remove_fatal_error_handler();
}

std::string Interpreter::unique_identifier() {
  return fmt::format("__CPPP_PRINTER_{}", ++id);
}

std::string Interpreter::wrap(llvm::StringRef block,
                              llvm::StringRef name) const {
  return fmt::format(R"(
extern "C" void {}(void* ret){{
  auto result = [](){};
  std::ostringstream stream;
  stream << result();
  *(static_cast<std::string*>(ret)) = stream.str();
}})",
                     name, block);
}

std::string Interpreter::eval(llvm::StringRef expression) {
  auto name = unique_identifier();
  std::string block;
  if (expression[0] != '{')
    block = fmt::format("{{ return {}; }}", expression);
  else
    block = expression;

  run(wrap(block, name));

  auto ret     = std::string{};
  auto printer = get<void(void*)>(name);

  if (!printer) {
    llvm::errs() << "Failed to print " << name << '\n';
    return {};
  }
  printer(&ret);
  return ret;
}

void Interpreter::link(llvm::StringRef path) {
  if (!path.contains(".so"))
    return;

  std::string err;
  auto failed = llvm::sys::DynamicLibrary::LoadLibraryPermanently(
      path.str().c_str(), &err);
  if (failed)
    llvm::errs() << err << '\n';
}

void Interpreter::run(llvm::StringRef code) {
  llvm::errs() << code;
  if (auto err = interpreter->ParseAndExecute(code)) {
    llvm::logAllUnhandledErrors(std::move(err), llvm::errs(), "error: ");
  }
}

void Interpreter::error_handler(void* userdata,
                                const char* message,
                                bool crash_diagnostics) {
  auto& diagnostics = *static_cast<clang::DiagnosticsEngine*>(userdata);
  diagnostics.Report(clang::diag::err_fe_error_backend) << message;
  llvm::sys::RunInterruptHandlers();

  // Sourced from clang-repl:
  //   We cannot recover from llvm errors.  When reporting a fatal error, exit
  //   with status 70 to generate crash diagnostics.  For BSD systems this is
  //   defined as an internal software error. Otherwise, exit with status 1.
  exit(crash_diagnostics ? 70 : 1);
}
