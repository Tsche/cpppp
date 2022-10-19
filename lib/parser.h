#pragma once
#include <list>
#include <memory>
#include <optional>
#include <tuple>
#include <variant>

#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>

#include "lexemes.h"
#include "interpreter.h"
#include "token.h"

class Parser {
public:
  using UnevaluatedID = size_t;

  Parser(std::unique_ptr<llvm::MemoryBuffer> input, Interpreter& interpreter)
      : input(std::move(input)),
        interpreter(interpreter) {}

  void parse();
  void evaluate(llvm::raw_ostream& output = llvm::outs());

private:
  std::unique_ptr<llvm::MemoryBuffer> input;
  Interpreter& interpreter;
  CodeGen codegen{};
  size_t temp_counter = 0;

  std::list<std::variant<llvm::StringRef, size_t>> outputs;

  auto extract(Token& _token)
      -> std::tuple<std::unique_ptr<Lexeme>, std::optional<Token>>;
};
