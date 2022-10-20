#pragma once
#include <optional>
#include <fmt/core.h>
#include <llvm/ADT/StringRef.h>
#include "codegen.h"

struct Lexeme {
  using UnevaluatedID = std::optional<size_t>;

  virtual ~Lexeme() {}

  virtual UnevaluatedID generate(CodeGen& code) = 0;
  virtual void print()                          = 0;
};

struct NullStatement : Lexeme {
  explicit NullStatement(llvm::StringRef = {}) {}
  ~NullStatement() override {}

  UnevaluatedID generate(CodeGen& code) override { return {}; }
  void print() override { llvm::errs() << "Null Statement\n"; }
};

struct Code : Lexeme {
  llvm::StringRef input;

  explicit Code(llvm::StringRef input) : input(input) {}
  ~Code() override {}

  UnevaluatedID generate(CodeGen& code) override;
  void print() override { llvm::errs() << "Code: " << input << '\n'; }
};

struct Expression : Code {
  explicit Expression(llvm::StringRef input) : Code(input) {}
  ~Expression() override {}

  UnevaluatedID generate(CodeGen& code) override;
  void print() override { llvm::errs() << "Expression: " << input << '\n'; }
};

struct Include : Code {
  explicit Include(llvm::StringRef input) : Code(input) {}
  ~Include() override {}

  UnevaluatedID generate(CodeGen& code) override;
  void print() override { llvm::errs() << "Include: " << input << '\n'; }
};

struct Function : Lexeme {
  llvm::StringRef name;
  llvm::StringRef block;

  explicit Function(llvm::StringRef name, llvm::StringRef block)
      : name(name),
        block(block) {}
  ~Function() override {}

  UnevaluatedID generate(CodeGen& code) override;
  void print() override {
    llvm::errs() << "Function: " << name << ' ' << block << '\n';
  }
};

struct Global : Lexeme {
  llvm::StringRef name;
  llvm::StringRef expression;

  explicit Global(llvm::StringRef name, llvm::StringRef expression)
      : name(name),
        expression(expression) {}
  ~Global() override {}

  UnevaluatedID generate(CodeGen& code) override;
  void print() override {
    llvm::errs() << "Global: " << name << '=' << expression << '\n';
  }
};
