#include "parser.h"
#include <clang/Basic/CharInfo.h>
#include <fmt/core.h>

size_t find_tokens(llvm::StringRef code,
                   llvm::StringRef tokens,
                   size_t start    = 0,
                   char next_token = 0) {
  char literal = 0;
  // TODO comments
  // TODO raw string literals
  for (auto i = start; i < code.size(); ++i) {
    switch (auto&& c = code[i]; c) {
      case '\\':
        if (i + 1 < code.size())
          i += code[i + 1] == '$' || code[i + 1] == '\n';
        break;

      case '"':
        [[fallthrough]];
      case '\'':
        literal = literal == c ? 0 : literal ? literal : c;
        break;

      default:
        if (!literal && tokens.contains(c)) {
          if (next_token) {
            if (code.size() < i + 1)
              return 0;  // none found

            if (code[i + 1] != next_token)
              continue;
          }
          return i + bool(next_token);
        }
    };
  }
  return 0;
}

size_t find_balanced(llvm::StringRef code,
                     char open,
                     char close,
                     size_t start = 0) {
  int count           = code[start] == open;
  char literal        = 0;
  char const tokens[] = {open, close};

  for (auto i = start + count; auto next = find_tokens(code, tokens, i);
       i                                 = ++next) {
    count += code[next] == open;
    count -= code[next] == close;
    if (!count)
      return next + 1;
  }
  return start;
}

size_t skip_whitespace(llvm::StringRef code, size_t start = 0) {
  auto skip = code.find_if_not(clang::isHorizontalWhitespace, start);
  return (skip == code.npos) ? start : skip;
}

size_t get_identifier(llvm::StringRef code, size_t start = 0) {
  if (clang::isAsciiIdentifierStart(code[0])) {
    auto ident = code.find_if_not(
        [](char c) { return clang::isAsciiIdentifierContinue(c); });

    return (ident == code.npos) ? code.size() : ident;
  }
  return start;
}

void Parser::parse(llvm::raw_ostream& output) {
  auto code = input->getBuffer();
  auto i    = 0;

  while (auto end = find_tokens(code, "$", i)) {
    output << code.substr(i, end - i);  // prelude
    auto temp = code.substr(++end);
    i         = end;

    if (temp[0] == '=') {
      i += find_tokens(temp, "$", 1);
      auto expression = code.substr(end + 1, i - end - 1);
      llvm::outs() << interpreter.eval(expression);
      continue;
    }
    if (temp[0] == '{') {
      i += find_balanced(temp, '{', '}');
      auto block = code.substr(end, i - end);
      llvm::outs() << interpreter.eval(block);
      continue;
    };

    if (temp.startswith("link ")) {
      i += find_tokens(temp, "\n");
      auto path = temp.substr(0, i - end);
      //TODO relative "" sys <>
      interpreter.link(path);
      continue;
    }

    if (temp.startswith("include ")) {
      i += find_tokens(temp, "\n");
      auto include   = temp.substr(0, i - end);
      auto generated = fmt::format("#{}\n", include);
      interpreter.run(generated);
      continue;
    }

    if (temp.startswith("define ")) {
      i += 7;
      auto command = temp.substr(7);
      auto pos     = get_identifier(command);
      auto name    = command.substr(0, pos);
      pos          = skip_whitespace(command, pos);

      if (command[pos] != '(') {
        end       = find_tokens(command, "\n");
        auto expr = command.substr(pos, end - pos);

        auto generated = fmt::format("auto {} = {};", name, expr);
        interpreter.run(generated);
      } else {
        end       = find_balanced(command, '(', ')', pos);
        end       = find_balanced(command, '{', '}', end);
        auto body = command.substr(pos, end - pos);

        auto generated = fmt::format("auto {}{}", name, body);
        interpreter.run(generated);
      }
      i += end;
      continue;
    }

    if (auto pos = get_identifier(temp)) {
      auto identifier = temp.substr(0, pos);
      end             = skip_whitespace(temp, pos);

      if (temp[end] != '(') {
        llvm::outs() << interpreter.eval(identifier);
      } else {
        end             = find_balanced(temp, '(', ')', end);
        auto expression    = temp.substr(0, end);
        llvm::outs() << interpreter.eval(expression);
      }
      i += end;
    }
  }

  // remaining code with no preprocessing tokens in it
  output << code.substr(i);
}
