#include <iostream>
@include <algorithm>
@include <vector>
@include <string_view>
@include <map>
@define bar(std::vector<int>&& v){
    return *std::min_element(v.begin(), v.end());
}
@define test(std::string_view a) {
  return a.substr(2);
}
@define foo test("oof")
@-
  double lefoo(){
    return 234.2;
  }
-@

int main(){
  auto bar = @bar({5, 1, 3, 2});
  auto fnc = @test("fo\"ooof\"");
  auto fnc2 = @lefoo();
  auto var = @foo;
  auto shorthand = @=std::string("\"lel\"")@;
  auto block = @{
    char a = 20 + 11 * 3;
    std::map<int, std::string> test {{3, "foo"}, {1, "bar"}};

    return test[3];
  };

  auto f1 = "@oof";
  @
  auto f2 = '@';
  auto f3 = "'@";
}

//@foo
//foo @far
/* @foo
 //"@foo" */
