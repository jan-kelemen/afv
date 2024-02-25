#include <afvbuf_text_buffer.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <string_view>
#include <type_traits>

// IWYU pragma: no_include <functional>

TEST_CASE("afv::buf::text_buffer construction")
{
    using namespace std::string_view_literals;

    using text_buffer = afv::buf::text_buffer;
    SECTION("default ctor")
    {
        text_buffer const buffer;
        REQUIRE(buffer.empty());
    }

    STATIC_REQUIRE(std::is_default_constructible_v<text_buffer>);
    STATIC_REQUIRE(std::is_copy_constructible_v<text_buffer>);
    STATIC_REQUIRE(std::is_nothrow_constructible_v<text_buffer>);
    STATIC_REQUIRE(std::is_copy_assignable_v<text_buffer>);
    STATIC_REQUIRE(std::is_move_assignable_v<text_buffer>);

    STATIC_REQUIRE(
        std::is_nothrow_move_assignable_v<afv::buf::basic_text_buffer<char,
            std::char_traits<char>,
            std::allocator<char>>>);
}

TEST_CASE("afv:::buf::basic_text_buffer insertion")
{
    using namespace std::string_view_literals;

    using text_buffer = afv::buf::text_buffer;
    SECTION("insert() on empty buffer")
    {
        text_buffer buffer;
        buffer.insert(0, "abc"sv);

        REQUIRE(std::ranges::equal(buffer, "abc"sv));
    }

    SECTION("insert() in middle of node")
    {
        text_buffer buffer;
        buffer.insert(0, "abc"sv);
        buffer.insert(1, "def"sv);

        REQUIRE(std::ranges::equal(buffer, "adefbc"sv));
    }

    SECTION("insert() at end of node")
    {
        text_buffer buffer;
        buffer.insert(0, "abc"sv);
        buffer.insert(3, "def"sv);

        REQUIRE(std::ranges::equal(buffer, "abcdef"sv));
    }

    SECTION("insert() with newline character increases line count")
    {
        text_buffer buffer;

        REQUIRE(buffer.lines() == 0);

        buffer.insert(0, "a"sv);
        REQUIRE(buffer.lines() == 1);

        buffer.insert(1, "\n"sv);
        REQUIRE(buffer.lines() == 1);

        buffer.insert(2, "def"sv);
        REQUIRE(buffer.lines() == 2);

        buffer.insert(0, "\n"sv);
        REQUIRE(buffer.lines() == 3);
    }
}

TEST_CASE("afv::buf::basic_text_buffer iterators")
{
    using namespace std::string_view_literals;

    using text_buffer = afv::buf::text_buffer;
    SECTION("begin(), end() can be used to construct a container")
    {
        text_buffer buffer;
        buffer.insert(0, "abc"sv);
        buffer.insert(3, "def"sv);

        REQUIRE(std::string{buffer.begin(), buffer.end()} == "abcdef");

        text_buffer const& const_buffer{buffer};
        REQUIRE(
            std::string{const_buffer.begin(), const_buffer.end()} == "abcdef");
    }

    SECTION("cbegin(), cend() can be used to construct a container")
    {
        text_buffer buffer;
        buffer.insert(0, "abc"sv);
        buffer.insert(3, "def"sv);

        text_buffer const& const_buffer{buffer};
        REQUIRE(std::string{const_buffer.cbegin(), const_buffer.cend()} ==
            "abcdef");
    }
}

TEST_CASE("afv::buf::basic_text_buffer lines")
{
    using namespace std::string_view_literals;

    using text_buffer = afv::buf::text_buffer;
    SECTION("line() returns whole content when document contains one line")
    {
        text_buffer buffer;
        buffer.insert(0, "abcdef"sv);
        REQUIRE(std::ranges::equal("abcdef"sv, buffer.line(0)));
    }

    SECTION("line() recognizes \\n as line separator")
    {
        text_buffer buffer;
        buffer.insert(0, "abcdef"sv);

        buffer.insert(3, "\n"sv);

        REQUIRE(std::ranges::equal("abc"sv, buffer.line(0)));
        REQUIRE(std::ranges::equal("def"sv, buffer.line(1)));
    }

    SECTION("line() returns empty range when document ends with newline")
    {
        text_buffer buffer;

        buffer.insert(0, "abc\n"sv);

        REQUIRE(buffer.lines() == 1);
        REQUIRE(std::ranges::equal(""sv, buffer.line(1)));
    }
}
