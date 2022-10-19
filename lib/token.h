#pragma once
#include <memory>
#include <optional>

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSwitch.h>

#include "lexemes.h"

class Token {
public:
  size_t start      = 0;
  size_t end        = 0;
  size_t offset_end = 0;

  explicit Token(llvm::StringRef input, size_t offset = 0);
  Token& skip(size_t amount);
  Token& skip_whitespace();

  Token& truncate(size_t amount);
  Token& drop_front(size_t amount);
  Token& drop_back(size_t amount);
  Token& drop_newline();

  Token& find_unescaped(char c);
  Token& find(llvm::StringRef needles, llvm::StringRef followed_by = {});
  Token& balance(char open, char close);
  Token& identifier();

  std::optional<Token> split(int offset = 0) const;

  template <typename T>
  auto switch_() {
    return llvm::StringSwitch<T>(input.substr(start));
  }

  template <typename T>
  auto lex() const
      -> std::tuple<std::unique_ptr<Lexeme>, std::optional<Token>> {
    if (has_error)
      return {std::make_unique<NullStatement>(), Token(input.substr(end))};

    return {std::make_unique<T>(str()), split()};
  }

  template <typename T>
  auto lex(Token& first) const
      -> std::tuple<std::unique_ptr<Lexeme>, std::optional<Token>> {
    if (has_error)
      return lex<NullStatement>();

    return {std::make_unique<T>(first.str(), str()), split()};
  }

  char front() const;
  char next() const;
  size_t empty() const { return start == end; }
  operator bool() const { return not has_error; }
  llvm::StringRef str(int offset = 0) const;

private:
  llvm::StringRef input;
  bool has_error = false;
};
