#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <llvm/Support/raw_ostream.h>


struct CodeGen {
  constexpr static std::string_view prefix = "__CPPPP_PRINTER_";
  using printer_t                          = std::string(void);
  std::vector<printer_t*> printers;

  std::string output = R"(
#include <string>
#include <sstream>
  )";

  llvm::raw_string_ostream stream{output};
};
