#include "token.h"
#include <clang/Basic/CharInfo.h>

#define CHECK_ERROR() \
  if (has_error) {    \
    return *this;     \
  }

#define SET_ERROR(condition) \
  if ((condition)) {         \
    has_error = true;        \
    return *this;            \
  }

Token::Token(llvm::StringRef input, size_t offset) : input(input) {
  if (input.empty()) {
    has_error = true;
  } else {
    end = start = offset;

    if (offset >= input.size()) {
      end = start = input.size() - 1;
      has_error   = true;
    }
  }
}

Token& Token::skip(size_t amount) {
  CHECK_ERROR();

  if (end + amount >= input.size()) {
    end = input.size() - 1;
    SET_ERROR(true);
  }

  end += amount;
  return *this;
}

Token& Token::skip_whitespace() {
  CHECK_ERROR();

  if (auto skip = input.find_if_not(clang::isHorizontalWhitespace, end);
      skip != input.npos) {
    end = skip;
  }

  return *this;
}

Token& Token::truncate(size_t amount) {
  CHECK_ERROR();

  // TODO bounds checking
  end -= amount;
  return *this;
}

Token& Token::drop_front(size_t amount) {
  CHECK_ERROR();

  // TODO bounds checking
  if (start == end) {
    end += amount;
  }

  start += amount;
  return *this;
}

Token& Token::drop_back(size_t amount) {
  CHECK_ERROR();

  // TODO bounds checking
  offset_end += amount;
  return *this;
}

Token& Token::drop_newline() {
  CHECK_ERROR();

  auto idx = offset_end + end;
  if (idx < input.size()) {
    if (clang::isVerticalWhitespace(input[idx]))
      ++offset_end;
  }
  return *this;
}

Token& Token::find(llvm::StringRef needles, llvm::StringRef followed_by) {
  CHECK_ERROR();

  for (; end < input.size(); ++end) {
    switch (auto&& c = input[end]; c) {
      case '\\':
        end += 2;
        continue;

      case '"':
        [[fallthrough]];
      case '\'':
        return find_unescaped(c).skip(1).find(needles, followed_by);

      case '/':
        if (input.size() < ++end)
          continue;

        switch (input[end]) {
          case '/':
            return find_unescaped('\n').skip(1).find(needles, followed_by);
          case '*':
            end = input.find("*/", ++end);
            continue;
        };
    };

    if (needles.contains(input[end])) {
      if (!followed_by.empty()) {
        SET_ERROR(input.size() < end + followed_by.size());

        if (not input.substr(end + 1).startswith(followed_by))
          continue;
      }

      end += followed_by.size();
      return *this;
    }
  }

  SET_ERROR(true);
}

Token& Token::balance(char open, char close) {
  CHECK_ERROR();

  int count           = 1;
  char const tokens[] = {open, close};

  while (skip(1).find(tokens)) {
    count += input[end] == open;
    count -= input[end] == close;

    if (!count) {
      ++end;
      return *this;
    }
  }

  SET_ERROR(true);
}

Token& Token::identifier() {
  CHECK_ERROR();

  SET_ERROR(!clang::isAsciiIdentifierStart(input[end]));

  auto ident = input.find_if_not(
      [](auto c) { return clang::isAsciiIdentifierContinue(c); },
      end);

  end = ident != input.npos ? ident : input.size() - 1;
  return *this;
}

Token& Token::find_unescaped(char c) {
  CHECK_ERROR();

  auto i = end;
  do {
    i = input.find(c, ++i);
    SET_ERROR(i == input.npos);
  } while (input[i - 1] == '\\');

  end = i;
  return *this;
}

std::optional<Token> Token::split(int offset) const {
  if (has_error)
    return {};

  auto next_start = offset_end + end + offset;
  if (next_start >= input.size() || next_start < 0)
    return {};

  return Token(input, next_start);
}

char Token::front() const {
  if (has_error)
    return '\0';

  return input[start];
}

char Token::next() const {
  auto next_start = offset_end + end;

  if (has_error || next_start >= input.size())
    return '\0';

  return input[next_start];
}

llvm::StringRef Token::str(int offset) const {
  auto next_start = end - offset;

  if (next_start >= input.size() || next_start < 0)
    return input.substr(start);

  return input.substr(start, next_start - start);
}
