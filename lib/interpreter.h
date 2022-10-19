#pragma once
#include <string>

#include <clang/Interpreter/Interpreter.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <clang/AST/GlobalDecl.h>

#include <llvm/Support/raw_ostream.h>
#include <unordered_map>

#include "codegen.h"

class Interpreter {
public:
  using printer_t = std::string(void);

  explicit Interpreter(std::vector<const char*>& args);
  ~Interpreter();
  void link(llvm::StringRef path);
  void run(CodeGen& code);


  template <typename T>
  T* get(llvm::StringRef name) const {
    auto addr = interpreter->getSymbolAddress(name);
    if (not addr)
      return nullptr;

    return llvm::jitTargetAddressToPointer<T*>(*addr);
  }

  template <typename T>
  T* get(clang::GlobalDecl decl) const {
    auto addr = interpreter->getSymbolAddress(decl);
    if (not addr)
      return nullptr;

    return llvm::jitTargetAddressToPointer<T*>(*addr);
  }

private:
  std::unique_ptr<clang::Interpreter> interpreter;
  static void error_handler(void* userdata,
                            const char* message,
                            bool crash_diagnostics);
};
