#include <sysexits.h>

#include <clang/Basic/Version.h>
#include <clang/Format/Format.h>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ManagedStatic.h>  //llvm_shutdown_obj
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>

// workaround for broken include path
#include <fmt/core.h>
#include "clang/Driver/Driver.h"

#include "interpreter.h"
#include "parser.h"
#include "version.h"

static llvm::cl::OptionCategory Category(PROJECT " options");

void VersionPrinter(llvm::raw_ostream& stream) {
  stream << PROJECT << " version " << VERSION << '\n'
         << clang::getClangFullVersion() << '\n';
}

llvm::raw_ostream& get_stream(std::string outpath) {
  return llvm::outs();
}

int main(int argc, char const* argv[]) {
  llvm::llvm_shutdown_obj _shutdown;
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  // Command line option definitions
  llvm::cl::opt<std::string> output(
      "output",
      llvm::cl::desc(
          "Output path. Should be a directory if multiple files have "
          "been specified."),
      llvm::cl::value_desc("path"), llvm::cl::cat(Category));

  llvm::cl::alias output_alias("o", llvm::cl::desc("Alias for --output"),
                               llvm::cl::aliasopt(output));

  llvm::cl::list<std::string> inputs(llvm::cl::Positional,
                                     llvm::cl::desc("[<file> ...]"),
                                     llvm::cl::cat(Category));

  // Parse command line options
  llvm::cl::HideUnrelatedOptions(Category);
  llvm::cl::SetVersionPrinter(VersionPrinter);
  llvm::cl::ParseCommandLineOptions(argc, argv, "<description>", nullptr,
                                    nullptr, true);

  // HACK resource-dir defaults to a relative path for some reason
  auto resc =
      fmt::format("-resource-dir={}",
                  clang::driver::Driver::GetResourcesPath("/bin/clang"));

  std::vector<const char*> clang_args{"-std=c++20", resc.c_str()};

  if (inputs.empty()) {
    llvm::errs() << "No inputs given\n";
    return EX_NOINPUT;
  }

  Interpreter interpreter{clang_args};

  for (auto& input : inputs) {
    auto in = llvm::MemoryBuffer::getFileOrSTDIN(input);
    if (std::error_code error = in.getError()) {
      llvm::errs() << error.message() << '\n';
      return error.value();
    }
    llvm::raw_ostream& out = get_stream(output);
    Parser p(std::move(*in), interpreter);
    p.parse(out);
  }

  return EX_OK;
}
