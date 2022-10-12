#pragma once
#include <memory>

#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>

#include "interpreter.h"

class Parser {
public:
  Parser(std::unique_ptr<llvm::MemoryBuffer> input,
         Interpreter& interpreter)
      : input(std::move(input)),
        interpreter(interpreter) {}

  void parse(llvm::raw_ostream& output = llvm::outs());

private:
  std::unique_ptr<llvm::MemoryBuffer> input;
  Interpreter& interpreter;
  size_t temp_counter = 0;
};
