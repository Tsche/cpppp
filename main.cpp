#include <llvm/Support/CommandLine.h>
#include "clang/Basic/Version.h"

static llvm::cl::OptionCategory Category(PROJECT "options");

void VersionPrinter(llvm::raw_ostream& stream) {
  stream 
    << PROJECT << ' ' << VERSION << '\n'
    << clang::getClangFullCPPVersion() << '\n';
}

int main(int argc, const char **argv){
  llvm::cl::HideUnrelatedOptions(Category);
  llvm::cl::SetVersionPrinter(VersionPrinter);
  llvm::cl::ParseCommandLineOptions(argc, argv);
}