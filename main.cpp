#include <sysexits.h>

#include <clang/Basic/Version.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ManagedStatic.h>  //llvm_shutdown_obj
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>

// workaround for broken include path
#include <clang/Driver/Driver.h>
#include <fmt/core.h>

#include <interpreter.h>
#include <parser.h>
#include <version.h>

static void VersionPrinter(llvm::raw_ostream& stream) {
  stream << PROJECT << " version " << VERSION << '\n'
         << clang::getClangFullVersion() << '\n';
}

static llvm::raw_ostream& get_stream(std::string outpath) {
  // TODO
  return llvm::outs();
}

int main(int argc, char const* argv[]) {
  // init llvm
  llvm::llvm_shutdown_obj _shutdown;
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  // Command line option definitions
  llvm::cl::OptionCategory category(PROJECT " options");
  llvm::cl::opt<std::string> output(
      "output",
      llvm::cl::desc(
          "Output path. Should be a directory if multiple files have "
          "been specified."),
      llvm::cl::value_desc("path"),
      llvm::cl::cat(category));

  llvm::cl::alias output_alias("o",
                               llvm::cl::desc("Alias for --output"),
                               llvm::cl::aliasopt(output));

  llvm::cl::opt<bool> print_parsed(
      "print-parsed",
      llvm::cl::desc(
          "Output the parsing result to stderr. Useful for debugging."),
      llvm::cl::cat(category));

  llvm::cl::opt<bool> print_program(
      "print-program",
      llvm::cl::desc(
          "Prints the generated program. Useful for debugging."),
      llvm::cl::cat(category));

  llvm::cl::opt<std::string> input(llvm::cl::Positional,
                                   llvm::cl::desc("<file>"),
                                   llvm::cl::init("-"),
                                   llvm::cl::cat(category));

  // Parse command line options
  llvm::cl::HideUnrelatedOptions(category);
  llvm::cl::SetVersionPrinter(VersionPrinter);
  llvm::cl::ParseCommandLineOptions(argc,
                                    argv,
                                    "<description>",
                                    nullptr,
                                    nullptr,
                                    true);

  // HACK resource-dir defaults to a relative path for some reason
  auto resc =
      fmt::format("-resource-dir={}",
                  clang::driver::Driver::GetResourcesPath("/bin/clang"));

  std::vector<const char*> clang_args{"-std=c++20", resc.c_str()};
  Interpreter interpreter{clang_args};

  auto in = llvm::MemoryBuffer::getFileOrSTDIN(input);
  if (std::error_code error = in.getError()) {
    llvm::errs() << error.message() << '\n';
    return error.value();
  }

  llvm::raw_ostream& out = get_stream(output);
  Parser parser(std::move(*in), interpreter);
  parser.parse(print_parsed);
  parser.evaluate(out, print_program);

  return EX_OK;
}
