#pragma once
#include <string>

#include <clang/Interpreter/Interpreter.h>
#include <llvm/ExecutionEngine/JITSymbol.h>

class Interpreter {
public:
  explicit Interpreter(std::vector<const char*>& args);
  ~Interpreter();

  std::string unique_identifier();
  std::string wrap(llvm::StringRef block, llvm::StringRef name) const;
  std::string eval(llvm::StringRef block);

  void link(llvm::StringRef path);
  void run(llvm::StringRef code);

  template <typename T>
  T* get(llvm::StringRef name) const {
    auto addr = interpreter->getSymbolAddress(name);
    if (not addr)
      return nullptr;

    return llvm::jitTargetAddressToPointer<T*>(*addr);
  }

private:
  unsigned id = 0;
  std::unique_ptr<clang::Interpreter> interpreter;

  static void error_handler(void* userdata,
                            const char* message,
                            bool crash_diagnostics);
};
