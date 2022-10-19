#include "parser.h"
#include <fmt/core.h>

auto Parser::extract(Token& token)
    -> std::tuple<std::unique_ptr<Lexeme>, std::optional<Token>> {
  using ret_t = std::tuple<std::unique_ptr<Lexeme>, std::optional<Token>>;
  using ret_f = ret_t (*)(Token & token);

  switch (token.front()) {
    case '@':
      return token.drop_back(1).drop_newline().lex<NullStatement>();
    case '=':
      return token.drop_front(1).find("@").drop_back(1).lex<Expression>();
    case '{':
      return token.balance('{', '}').drop_newline().lex<Expression>();
    case '-':
      return token.drop_front(1)
          .find("-", "@")
          .truncate(1)
          .drop_back(2)
          .drop_newline()
          .lex<Code>();
  };

  auto fnc =
      token.switch_<ret_f>()
          .StartsWith("include ",
                      [](Token& token) {
                        return token.find("\n").drop_newline().lex<Include>();
                      })
          .StartsWith("define ",
                      [](Token& token) -> ret_t {
                        auto& name = token.drop_front(7).identifier();
                        auto temp  = name.split();

                        if (!temp.has_value())
                          return temp->lex<NullStatement>();

                        if (temp->front() != '(')
                          return temp->skip_whitespace()
                              .find("\n")
                              .drop_newline()
                              .lex<Global>(name);
                        else
                          return temp->balance('(', ')')
                              .skip_whitespace()
                              .balance('{', '}')
                              .drop_newline()
                              .lex<Function>(name);
                      })
          .Default([](Token& token) -> ret_t {
            auto& identifier = token.identifier();

            if (!identifier)
              return identifier.lex<NullStatement>();

            if (identifier.next() != '(')
              return identifier.lex<Expression>();
            else
              return identifier.skip_whitespace()
                  .balance('(', ')')
                  .lex<Expression>();
          });
  return fnc(token);
}

void Parser::parse() {
  auto code = Token(input->getBuffer()).find("@");

  if (!code) {
    // code does not contain any tokens
    outputs.push_back(input->getBuffer());
    return;
  }

  if (auto prelude_str = code.str(); !prelude_str.empty()) {
    // code before current token
    outputs.push_back(prelude_str);
  }

  for (auto token = code.split(1); token.has_value();) {
    auto [lexeme, prelude] = extract(*token);

    lexeme->print();

    if (auto unevaluated = lexeme->generate(codegen); unevaluated) {
      // unevaluated tag -> evaluate later
      outputs.push_back(*unevaluated);
    }

    if (!prelude.has_value()) {
      // TODO error state
      llvm::errs()
          << "Something went terribly wrong, nothing follows the last token.\n";
      break;
    }

    prelude->find("@");
    if (!(*prelude)) {
      // code after last token
      outputs.push_back(prelude->str());
      break;
    }

    if (auto prelude_str = prelude->str(); !prelude->empty()) {
      // code before next token
      outputs.push_back(prelude_str);
    }

    token = prelude->split(1);
  }
}

void Parser::evaluate(llvm::raw_ostream& output) {
  interpreter.run(codegen);

  for (auto&& chunk : outputs) {
    std::visit(
        [&output, this](auto&& element) {
          using T = std::decay_t<decltype(element)>;
          if constexpr (std::is_same_v<T, llvm::StringRef>)
            output << element;
          if constexpr (std::is_same_v<T, UnevaluatedID>) {
            if (codegen.printers.size() < element)
              return;

            if (auto* printer = codegen.printers[element]; printer)
              output << (*printer)();
          } else
            return;
        },
        chunk);
  }
}
