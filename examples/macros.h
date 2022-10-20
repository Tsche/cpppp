#include <iostream>
@-
#define bruhify(x) "#define bruh " #x
-@

@define bruh() {
  return bruhify("what");
}
@bruh()

int main(){
  std::cout << bruh;
}
