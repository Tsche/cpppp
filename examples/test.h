#include <iostream>
$include <string_view>

$define test(std::string_view a) {
  return a.substr(2);
}

$define foo test("oof")

int main(){
  auto fnc = $test("foooof");
  auto var = $foo;
  auto shorthand = $=std::string("lel")$;
  auto block = ${
    char a = 20 + 11 * 3;
    std::map<int, std::string> test {{3, "foo"}, {1, "bar"}};

    return test[3];
  };

  auto f1 = "$oof";
  auto f2 = '$';
  auto f3 = "'$";
}
