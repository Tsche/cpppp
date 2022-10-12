#include <iostream>

$link ../todo.so
$include <algorithm>

%-
inline code block // TODO
-%

$define test(const char* a) {
  return a;
}
$define foo test("oof" "(){}")

int main(){
  auto fnc = $test("a");
  auto var = $foo;
  auto shorthand = $=std::string("foobar")$;
  auto block = ${
    char a = 3*15;
    return std::string("foo");
  };

  auto f1 = "$oof";
  auto f2 = '$';
  auto f3 = "'$";
}
