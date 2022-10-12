#pragma once
// If target_compile_definitions is set correctly and project specifies a
// version these macros will do nothing. This file only exists to stop the
// linter from bitching about possibly undefined identifiers.

#ifndef PROJECT
  #define PROJECT "<project>"
#endif

#ifndef VERSION
  #define VERSION ""
#endif
