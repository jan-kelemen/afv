#include <afvbuf_text_buffer.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("afv::buf::text_buffer construction")
{
    using namespace std::string_view_literals;

    using text_buffer = afv::buf::text_buffer;
    SECTION("default ctor")
    {
        text_buffer buffer;
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
