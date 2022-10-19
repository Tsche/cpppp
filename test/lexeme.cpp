#include <token.h>
#include <catch2/catch_test_macros.hpp>

auto get_token(llvm::StringRef input, int offset = 1) {
  auto prelude = Token(input).find("@");
  REQUIRE(prelude);
  auto token = prelude.split(offset);
  REQUIRE(token.has_value());
  return token;
}

TEST_CASE("shorthand expression") {
  llvm::StringRef input = R"(
auto foo = @="bar"@;)";
  auto token            = *get_token(input);

  auto block = token.find("@");
  REQUIRE(block);
}

TEST_CASE("bracket expression") {
  llvm::StringRef input = R"(
auto foo = @{
  foobar{};
};)";
  auto token            = *get_token(input);

  auto block = token.balance('{', '}');
  REQUIRE(block);
  REQUIRE(block.start == 13);
  REQUIRE(block.end == input.size() - 1);
  auto block_str = block.str();
  REQUIRE(block_str.equals(R"({
  foobar{};
})"));
}

TEST_CASE("inline code") {
  llvm::StringRef input = R"(
auto foo = "foo";
@-
  struct Foo{
    void bar();
  };
-@)";
  auto token            = *get_token(input);

  // TODO
  auto block = token.drop_front(1).find("-", "@").truncate(1);
  REQUIRE(block.str().equals(R"(
  struct Foo{
    void bar();
  };
)"));
  REQUIRE(block);
}

TEST_CASE("include directive") {
  llvm::StringRef input = R"(
@include <string>
)";
  auto token            = *get_token(input);

  auto block = token.find_unescaped('\n');
  REQUIRE(block);
  REQUIRE(block.str().equals("include <string>"));
}

TEST_CASE("define global") {
  llvm::StringRef input = R"(
@define bar "bar"
)";
  auto token            = *get_token(input);

  auto name = token.drop_front(7).identifier();
  REQUIRE(name);
  REQUIRE(name.str().equals("bar"));

  auto temp = name.split(0);
  REQUIRE(temp.has_value());

  auto expr = temp->skip_whitespace().find("\n").drop_newline();
  REQUIRE(expr);
  REQUIRE(expr.str().equals(" \"bar\""));
}

TEST_CASE("define function") {
  llvm::StringRef input = R"(
@define foo(auto x) {return x;})";
  auto token            = *get_token(input);

  auto name = token.drop_front(7).identifier();
  REQUIRE(name);
  REQUIRE(name.str().equals("foo"));

  auto temp = name.split(0);
  REQUIRE(temp.has_value());

  auto rest = temp->balance('(', ')').skip_whitespace().balance('{', '}');
  REQUIRE(rest.str().equals("(auto x) {return x;}"));
  REQUIRE(rest);
}

TEST_CASE("simple expression") {
  llvm::StringRef input = R"(
auto bar = @bar;
)";
  auto token            = *get_token(input);
  auto identifier       = token.identifier();
  REQUIRE(identifier);
  REQUIRE(identifier.str().equals("bar"));
}

TEST_CASE("function call expression") {
  llvm::StringRef input = R"(
auto foo = @foo(3);
)";
  auto token            = *get_token(input);
  auto expression = token.identifier().skip_whitespace().balance('(', ')');
  REQUIRE(expression);
  REQUIRE(expression.str().equals("foo(3)"));
}
