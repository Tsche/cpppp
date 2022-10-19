#include "interpreter.h"

#include <clang/Frontend/CompilerInstance.h>

#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/FrontendDiagnostic.h>

#include <llvm/Support/DynamicLibrary.h>  // LoadLibraryPermanently
#include <llvm/Support/Error.h>           // cantFail
#include <llvm/Support/Signals.h>         // RunInterruptHandlers
#include <llvm/Support/TargetSelect.h>    // llvm::Initialize*

#include <fmt/core.h>
#include <string>

#include <clang/AST/Decl.h>
#include <clang/AST/Mangle.h>

#include <charconv>

Interpreter::Interpreter(std::vector<const char*>& args) {
  auto compiler =
      llvm::cantFail(clang::IncrementalCompilerBuilder::create(args));

  llvm::install_fatal_error_handler(
      error_handler,
      static_cast<void*>(&compiler->getDiagnostics()));
  compiler->LoadRequestedPlugins();
  interpreter = llvm::cantFail(clang::Interpreter::create(std::move(compiler)));

  /*push(R"(
#include <string>
#include <sstream>
)");*/
}

Interpreter::~Interpreter() {
  llvm::remove_fatal_error_handler();
}

void Interpreter::link(llvm::StringRef path) {
  if (!path.contains(".so"))
    return;

  std::string err;
  auto failed =
      llvm::sys::DynamicLibrary::LoadLibraryPermanently(path.str().c_str(),
                                                        &err);
  if (failed)
    llvm::errs() << err << '\n';
}

void Interpreter::run(CodeGen& code) {
  auto PTU = interpreter->Parse(code.output);
  if (auto err = PTU.takeError()) {
    llvm::logAllUnhandledErrors(std::move(err),
                                llvm::errs(),
                                "Parsing error: ");
    return;
  }

  if (auto err = interpreter->Execute(*PTU)) {
    llvm::logAllUnhandledErrors(std::move(err), llvm::errs(), "error: ");
  }

  // Get printers
  for (auto&& decl : PTU->TUPart->decls()) {
    if (!decl->isFunctionOrFunctionTemplate())
      continue;

    auto* fnc = decl->getAsFunction();
    auto name = fnc->getQualifiedNameAsString();

    if (!name.starts_with(code.prefix))
      continue;

    auto id_string = std::string_view(name);
    id_string.remove_prefix(code.prefix.length());
    if (size_t id; std::from_chars(id_string.begin(), id_string.end(), id).ec ==
                   std::errc{}) {
      auto* printer = get<printer_t>(decl->getAsFunction());

      if (!printer)
        continue;

      if (code.printers.size() < id)
        continue;

      code.printers[id] = printer;
    }
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
