#include <token.h>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("empty input") {
  llvm::StringRef input = "";
  SECTION("empty") {
    auto token = Token(input);
    REQUIRE_FALSE(token);

    REQUIRE(token.start == token.end);
    REQUIRE(token.empty());

    auto token_offset = Token(input, 10);
    REQUIRE_FALSE(token_offset);

    REQUIRE(token_offset.empty());
    REQUIRE(token_offset.start == token_offset.end);
  }

  SECTION("skip") {
    auto token = Token(input).skip(10);
    REQUIRE_FALSE(token);

    REQUIRE(token.start == token.end);
    REQUIRE(token.empty());
  }

  SECTION("skip whitespace") {
    auto token = Token(input).skip_whitespace();
    REQUIRE_FALSE(token);

    REQUIRE(token.start == token.end);
    REQUIRE(token.empty());
  }

  SECTION("find_unescaped") {
    auto token = Token(input).find_unescaped('a');
    REQUIRE_FALSE(token);

    REQUIRE(token.start == token.end);
    REQUIRE(token.empty());
  }
}

TEST_CASE("whitespace") {
  SECTION("only spaces") {
    llvm::StringRef input = "  \t ";
    auto token            = Token(input).skip_whitespace();
    REQUIRE(token);

    REQUIRE(token.start == token.end);
    REQUIRE(token.empty());
  }

  SECTION("spaces before") {
    llvm::StringRef input = "  \t foo";
    auto token            = Token(input).skip_whitespace();
    REQUIRE(token);
    REQUIRE(token.start != token.end);
    REQUIRE(token.end == 4);
  }
}

TEST_CASE("skip") {
  llvm::StringRef input = "foobar";
  SECTION("valid skip") {
    auto token = Token(input).skip(3);
    REQUIRE(token);
    REQUIRE(token.end == 3);
  }

  SECTION("skip 0") {
    auto token = Token(input).skip(0);
    REQUIRE(token);
    REQUIRE(token.end == 0);
  }

  SECTION("skip past end") {
    auto token = Token(input).skip(10);
    REQUIRE_FALSE(token);
    REQUIRE(token.end == input.size() - 1);
  }
}

TEST_CASE("find_unescaped") {
  SECTION("before eof") {
    llvm::StringRef input = "abcdef\\nn";
    auto token            = Token(input).find_unescaped('n');
    REQUIRE(token);
    REQUIRE(token.end == input.size() - 1);
  }

  SECTION("with succeeding characters") {
    llvm::StringRef input = "a\\c\\nef\\nnasdf";
    auto token            = Token(input).find_unescaped('n');
    REQUIRE(token);
    REQUIRE(token.end == 9);
  }

  SECTION("not included") {
    llvm::StringRef input = "a\\nbcdef\\n";
    auto token            = Token(input).find_unescaped('n');
    REQUIRE_FALSE(token);
  }
}

TEST_CASE("split") {
  llvm::StringRef input = "foobar";

  SECTION("empty") {
    llvm::StringRef empty = "";
    auto token            = Token(empty);

    auto next = token.split(0);
    REQUIRE_FALSE(next.has_value());
  }

  SECTION("valid") {
    auto token = Token(input).skip(3);
    REQUIRE(token);

    auto next = token.split(0);
    REQUIRE(next.has_value());
    REQUIRE(token.end == 3);
    REQUIRE(next->start == token.end);
  }

  SECTION("with prior error") {
    auto token = Token(input).skip(10);
    REQUIRE_FALSE(token);

    auto next = token.split(0);
    REQUIRE_FALSE(next.has_value());
  }

  SECTION("truncate") {
    auto token = Token(input).skip(2);
    REQUIRE(token);

    auto next = token.split(2);
    REQUIRE(next.has_value());
    REQUIRE(token.end == 2);
    REQUIRE(next->start == 4);
  }

  SECTION("truncate past end") {
    auto token = Token(input).skip(3);
    REQUIRE(token);

    auto next = token.split(10);
    REQUIRE_FALSE(next.has_value());
  }
}

TEST_CASE("find") {
  SECTION("before eof") {
    llvm::StringRef input = "abcdefn";
    auto token            = Token(input).find("n");
    REQUIRE(token);
    REQUIRE(token.end == input.size() - 1);
  }

  SECTION("with succeeding characters") {
    llvm::StringRef input = "foobar";
    auto token            = Token(input).find("b");
    REQUIRE(token);
    REQUIRE(token.end == 3);
  }

  SECTION("not included") {
    llvm::StringRef input = "abcdef";
    auto token            = Token(input).find("n");
    REQUIRE_FALSE(token);
  }

  SECTION("escaped") {
    llvm::StringRef input = "foobar";
    auto token            = Token(input).find("b");
    REQUIRE(token);
    REQUIRE(token.end == 3);
  }

  SECTION("string or char") {
    llvm::StringRef input = "foo'b'ar\"b\"b";
    auto token            = Token(input).find("b");
    REQUIRE(token);
    REQUIRE(token.end == input.size() - 1);
  }

  SECTION("one-line comment") {
    llvm::StringRef input = "//foobar\nb";
    auto token            = Token(input).find("b");
    REQUIRE(token);
    REQUIRE(token.end == input.size() - 1);
  }

  SECTION("multi-line comment") {
    llvm::StringRef input = "/*foo\nbar*/b";
    auto token            = Token(input).find("b");
    REQUIRE(token);
    REQUIRE(token.end == input.size() - 1);
  }

  SECTION("escaped") {
    llvm::StringRef input = "foo\\barb";
    auto token            = Token(input).find("b");
    REQUIRE(token);
    REQUIRE(token.end == input.size() - 1);
  }
}

TEST_CASE("balance") {
  SECTION("one pair") {
    llvm::StringRef input = "()";
    auto token            = Token(input).balance('(', ')');
    REQUIRE(token);
    REQUIRE(token.end == input.size());
    REQUIRE(token.str().size() == input.size());
  }

  SECTION("one pair with succeeding characters") {
    llvm::StringRef input = "()asdf";
    auto token            = Token(input).balance('(', ')');
    REQUIRE(token);
    REQUIRE(token.end == 2);
  }

  SECTION("one pair missing opening character") {
    llvm::StringRef input = "as)asdf";
    auto token            = Token(input).balance('(', ')');
    REQUIRE(token);
    REQUIRE(token.end == 3);
  }
}

TEST_CASE("identifier") {
  SECTION("eof") {
    llvm::StringRef input = "foo";
    auto token            = Token(input).identifier();
    REQUIRE(token.end == input.size() - 1);
  }
  SECTION("terminated by newline") {
    llvm::StringRef input = "foo\na";
    auto token            = Token(input).identifier();
    REQUIRE(token.end == input.size() - 2);
  }

  SECTION("terminated by punctuation") {
    llvm::StringRef input = "foo;a";
    auto token            = Token(input).identifier();
    REQUIRE(token.end == input.size() - 2);
  }
}


