#include "lexemes.h"

Code::UnevaluatedID Code::generate(CodeGen& code) {
  code.stream << input;
  return {};
}

Code::UnevaluatedID Expression::generate(CodeGen& code) {
  std::string block =
      (input[0] == '{') ? input.str() : fmt::format("{{ return {}; }}", input);

  code.printers.push_back(nullptr);
  auto i = code.printers.size() - 1;

  code.stream << fmt::format(R"(
std::string {}{}(){{
  std::ostringstream stream;
  stream << [](){}();
  return stream.str();
}}
)",
                             code.prefix,
                             i,
                             block);
  return i;
}

Code::UnevaluatedID Include::generate(CodeGen& code) {
  code.stream << fmt::format("#{}\n", input);
  return {};
}


Code::UnevaluatedID Global::generate(CodeGen& code) {
  code.stream << fmt::format("auto {} = {};\n", name, expression);
  return {};
}

Code::UnevaluatedID Function::generate(CodeGen& code) {
  code.stream << fmt::format("auto {}{}\n", name, block);
  return {};
}
