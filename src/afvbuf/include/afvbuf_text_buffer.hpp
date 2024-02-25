#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <memory_resource>
#include <ranges>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

namespace afv::buf
{
    namespace detail
    {
        template<typename CharT, typename Traits, typename Allocator>
        using buffer = std::basic_string<CharT, Traits, Allocator>;

        struct node final
        {
            std::size_t buffer_index{};
            std::size_t start_offset{};
            std::size_t length{};
        };
    } // namespace detail

    template<typename CharT, typename Traits, typename Allocator>
    class basic_text_buffer_const_iterator;

    template<typename CharT, typename Traits, typename Allocator>
    class basic_text_buffer_iterator;

    template<typename CharT,
        typename Traits = std::char_traits<CharT>,
        typename Allocator = std::pmr::polymorphic_allocator<CharT>>
    class basic_text_buffer
    {
    public: // Types
        using value_type = CharT;
        using reference = value_type&;
        using traits_type = Traits;

        using allocator_type = Allocator;
        using difference_type =
            std::allocator_traits<Allocator>::difference_type;
        using size_type = std::allocator_traits<Allocator>::size_type;

        using iterator = basic_text_buffer_iterator<CharT, Traits, Allocator>;
        using const_iterator =
            basic_text_buffer_const_iterator<CharT, Traits, Allocator>;

    private:
        using buffer = detail::buffer<CharT, Traits, Allocator>;

        using buffer_allocator =
            std::allocator_traits<Allocator>::template rebind_alloc<buffer>;

        using buffer_container = std::vector<buffer, buffer_allocator>;

        using node_allocator = std::allocator_traits<
            Allocator>::template rebind_alloc<detail::node>;

        using node_container = std::vector<detail::node, node_allocator>;

    public: // Construction
        constexpr basic_text_buffer() noexcept(
            std::is_nothrow_constructible_v<Allocator> &&
            std::is_nothrow_constructible_v<buffer_container, Allocator> &&
            std::is_nothrow_constructible_v<node_container, Allocator>)
            : buffers_{Allocator{}}
            , nodes_{Allocator{}}
        {
        }

        constexpr explicit basic_text_buffer(Allocator const& alloc) noexcept(
            std::is_nothrow_constructible_v<buffer_container, Allocator> &&
            std::is_nothrow_constructible_v<node_container, Allocator>)
            : buffers_{alloc}
            , nodes_{alloc}
        {
        }

        constexpr basic_text_buffer(basic_text_buffer const&) noexcept(
            std::is_nothrow_copy_constructible_v<buffer_container> &&
            std::is_nothrow_copy_constructible_v<node_container>) = default;

        // clang-format off
        // NOLINTBEGIN(cppcoreguidelines-noexcept-move-operations, performance-noexcept-move-constructor)
        // clang-format on
        constexpr basic_text_buffer(basic_text_buffer&&) noexcept(
            std::is_nothrow_move_constructible_v<buffer_container> &&
            std::is_nothrow_move_constructible_v<node_container>) = default;
        // clang-format off
        // NOLINTEND(cppcoreguidelines-noexcept-move-operations, performance-noexcept-move-constructor)
        // clang-format on

    public: // Destruction
        ~basic_text_buffer() = default;

    public: // Interface
        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return nodes_.empty();
        }

        [[nodiscard]] constexpr size_type lines() const noexcept;

        [[nodiscard]] constexpr std::ranges::subrange<const_iterator> line(
            size_type line) const;

        template<std::ranges::forward_range Range>
        constexpr void insert(size_type position, Range const& range);

        template<std::forward_iterator Iterator,
            std::sentinel_for<Iterator> Sentinel>
        constexpr void insert(size_type position, Iterator begin, Sentinel end);

    public: // Iterators
        [[nodiscard]] constexpr iterator begin() noexcept
        {
            return {nodes_, 0, buffers_.data(), 0};
        }

        [[nodiscard]] constexpr iterator end() noexcept
        {
            return {nodes_, nodes_.size(), buffers_.data(), 0};
        }

        [[nodiscard]] constexpr const_iterator begin() const noexcept
        {
            return {nodes_, 0, buffers_.data(), 0};
        }

        [[nodiscard]] constexpr const_iterator end() const noexcept
        {
            return {nodes_, nodes_.size(), buffers_.data(), 0};
        }

        [[nodiscard]] constexpr const_iterator cbegin() const noexcept
        {
            return {nodes_, 0, buffers_.data(), 0};
        }

        [[nodiscard]] constexpr const_iterator cend() const noexcept
        {
            return {nodes_, nodes_.size(), buffers_.data(), 0};
        }

    public: // Operators
        constexpr basic_text_buffer&
        operator=(basic_text_buffer const&) noexcept(
            std::is_nothrow_copy_assignable_v<buffer_container> &&
            std::is_nothrow_copy_assignable_v<node_container>) = default;

        // clang-format off
        // NOLINTBEGIN(cppcoreguidelines-noexcept-move-operations, performance-noexcept-move-constructor)
        // clang-format on
        constexpr basic_text_buffer& operator=(basic_text_buffer&&) noexcept(
            std::is_nothrow_move_assignable_v<buffer_container> &&
            std::is_nothrow_move_assignable_v<node_container>) = default;
        // clang-format off
        // NOLINTEND(cppcoreguidelines-noexcept-move-operations, performance-noexcept-move-constructor)
        // clang-format on

    private: // Data
        buffer_container buffers_;
        node_container nodes_;
        size_type lines_{};
    };

    template<typename CharT, typename Traits, typename Allocator>
    constexpr basic_text_buffer<CharT, Traits, Allocator>::size_type
    basic_text_buffer<CharT, Traits, Allocator>::lines() const noexcept
    {
        if (empty())
        {
            return 0;
        }

        if (*std::prev(cend()) != '\n')
        {
            return lines_ + 1;
        }

        return lines_;
    }

    template<typename CharT, typename Traits, typename Allocator>
    constexpr std::ranges::subrange<
        basic_text_buffer_const_iterator<CharT, Traits, Allocator>>
    basic_text_buffer<CharT, Traits, Allocator>::line(
        basic_text_buffer::size_type line) const
    {
        auto begin{cbegin()};
        auto end{cend()};
        for (; begin != end && line > 0; ++begin)
        {
            if (*begin == '\n')
            {
                --line;
            }
        }
        return std::ranges::subrange(begin,
            std::ranges::find(begin, end, '\n'));
    }

    template<typename CharT, typename Traits, typename Allocator>
    template<std::ranges::forward_range Range>
    constexpr void basic_text_buffer<CharT, Traits, Allocator>::insert(
        basic_text_buffer::size_type position,
        Range const& range)
    {
        insert(position, std::ranges::begin(range), std::ranges::end(range));
    }

    template<typename CharT, typename Traits, typename Allocator>
    template<std::forward_iterator Iterator,
        std::sentinel_for<Iterator> Sentinel>
    constexpr void basic_text_buffer<CharT, Traits, Allocator>::insert(
        basic_text_buffer::size_type position,
        Iterator begin,
        Sentinel end)
    {
        size_type split_at{};
        auto const node_it{std::ranges::find_if(nodes_,
            [running_sum = size_type{}, position, &split_at](
                detail::node const& n) mutable noexcept
            {
                if (position >= running_sum &&
                    position < running_sum + n.length)
                {
                    split_at = position - running_sum;
                    return true;
                }
                running_sum += n.length;
                return false;
            })};

        auto const& text{buffers_.emplace_back(begin, end)};
        lines_ += static_cast<size_type>(std::ranges::count(text, '\n'));
        if (node_it == nodes_.cend()) // Appending to end
        {
            nodes_.emplace_back(buffers_.size() - 1, 0, text.size());
            return;
        }

        if (split_at == 0) // Add buffer before the current one
        {
            nodes_.insert(node_it,
                detail::node{.buffer_index = buffers_.size() - 1,
                    .start_offset = 0,
                    .length = text.size()});
            return;
        }

        // Split the existing node into two and new node in the middle
        detail::node const new_node{buffers_.size() - 1, 0, text.size()};
        detail::node const new_split{node_it->buffer_index,
            split_at,
            node_it->length - split_at};

        node_it->length = split_at;
        nodes_.insert(std::next(node_it), {new_node, new_split});
    }

    template<typename CharT, typename Traits, typename Allocator>
    class basic_text_buffer_const_iterator final
    {
    public: // Types
        using value_type = std::add_const_t<
            typename basic_text_buffer<CharT, Traits, Allocator>::value_type>;
        using element_type = value_type;
        using reference = value_type&;
        using pointer = value_type*;
        using difference_type =
            basic_text_buffer<CharT, Traits, Allocator>::difference_type;
        using iterator_category = std::bidirectional_iterator_tag;

    public: // Construction
        constexpr basic_text_buffer_const_iterator() = default;

        constexpr basic_text_buffer_const_iterator(
            std::span<detail::node const> nodes,
            basic_text_buffer<CharT, Traits, Allocator>::size_type node_index,
            detail::buffer<CharT, Traits, Allocator> const* buffers,
            basic_text_buffer<CharT, Traits, Allocator>::size_type buffer_index)
            : nodes_{nodes}
            , node_index_{node_index}
            , buffers_{buffers}
            , local_index_{buffer_index}
        {
        }

        constexpr basic_text_buffer_const_iterator(
            basic_text_buffer_const_iterator const&) noexcept = default;

        constexpr basic_text_buffer_const_iterator(
            basic_text_buffer_const_iterator&&) noexcept = default;

    public: // Destruction
        ~basic_text_buffer_const_iterator() = default;

    public: // Operators
        constexpr reference operator*() const noexcept
        {
            auto const& node{current_node()};
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            auto const& buffer{buffers_[node.buffer_index]};
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            auto const value_index{node.start_offset + local_index_};
            return buffer[value_index];
        }

        constexpr pointer operator->() const noexcept { return &(*(*this)); }

        constexpr basic_text_buffer_const_iterator& operator++() noexcept
        {
            auto const& node{current_node()};
            if (++local_index_ == node.length)
            {
                ++node_index_;
                local_index_ = 0;
            }

            return *this;
        }

        constexpr basic_text_buffer_const_iterator operator++(int) noexcept
        {
            auto tmp{*this};
            ++(*this);
            return tmp;
        }

        constexpr basic_text_buffer_const_iterator& operator--() noexcept
        {
            if (local_index_ == 0 && node_index_ > 0)
            {
                local_index_ = nodes_[--node_index_].length;
                // Intentional fallthrough
            }

            --local_index_;

            return *this;
        }

        constexpr basic_text_buffer_const_iterator operator--(int) noexcept
        {
            auto tmp{*this};
            --(*this);
            return tmp;
        }

        constexpr basic_text_buffer_const_iterator& operator=(
            basic_text_buffer_const_iterator const&) noexcept = default;

        constexpr basic_text_buffer_const_iterator& operator=(
            basic_text_buffer_const_iterator&&) noexcept = default;

        constexpr friend bool operator==(
            basic_text_buffer_const_iterator const& lhs,
            basic_text_buffer_const_iterator const& rhs) noexcept
        {
            return lhs.node_index_ == rhs.node_index_ &&
                lhs.local_index_ == rhs.local_index_;
        }

        constexpr friend bool operator!=(
            basic_text_buffer_const_iterator const& lhs,
            basic_text_buffer_const_iterator const& rhs) noexcept
        {
            return !(lhs == rhs);
        }

    private: // Helpers
        [[nodiscard]] constexpr detail::node const&
        current_node() const noexcept
        {
            return nodes_[node_index_];
        }

    private: // Data
        std::span<detail::node const> nodes_;
        size_t node_index_{};
        detail::buffer<CharT, Traits, Allocator> const* buffers_{};
        size_t local_index_{};
    };

    template<typename CharT, typename Traits, typename Allocator>
    class basic_text_buffer_iterator final
    {
    public: // Types
        using value_type =
            typename basic_text_buffer<CharT, Traits, Allocator>::value_type;
        using element_type = value_type;
        using reference = value_type&;
        using pointer = value_type*;
        using difference_type =
            basic_text_buffer<CharT, Traits, Allocator>::difference_type;
        using iterator_category = std::bidirectional_iterator_tag;

    public: // Construction
        constexpr basic_text_buffer_iterator() = default;

        constexpr basic_text_buffer_iterator(std::span<detail::node> nodes,
            basic_text_buffer<CharT, Traits, Allocator>::size_type node_index,
            detail::buffer<CharT, Traits, Allocator>* buffers,
            basic_text_buffer<CharT, Traits, Allocator>::size_type buffer_index)
            : nodes_{nodes}
            , node_index_{node_index}
            , buffers_{buffers}
            , local_index_{buffer_index}
        {
        }

        constexpr basic_text_buffer_iterator(
            basic_text_buffer_iterator const&) noexcept = default;

        constexpr basic_text_buffer_iterator(
            basic_text_buffer_iterator&&) noexcept = default;

    public: // Destruction
        ~basic_text_buffer_iterator() = default;

    public: // Operators
        constexpr reference operator*() const noexcept
        {
            auto const& node{current_node()};
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            auto& buffer{buffers_[node.buffer_index]};
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            auto const value_index{node.start_offset + local_index_};
            return buffer[value_index];
        }

        constexpr pointer operator->() const noexcept { return &(*(*this)); }

        constexpr basic_text_buffer_iterator& operator++() noexcept
        {
            auto const& node{current_node()};
            if (++local_index_ == node.length)
            {
                ++node_index_;
                local_index_ = 0;
            }

            return *this;
        }

        constexpr basic_text_buffer_iterator operator++(int) noexcept
        {
            auto tmp{*this};
            ++(*this);
            return tmp;
        }

        constexpr basic_text_buffer_iterator& operator--() noexcept
        {
            if (local_index_ == 0 && node_index_ > 0)
            {
                local_index_ = nodes_[--node_index_].length;
                // Intentional fallthrough
            }

            --local_index_;

            return *this;
        }

        constexpr basic_text_buffer_iterator operator--(int) noexcept
        {
            auto tmp{*this};
            --(*this);
            return tmp;
        }

        constexpr basic_text_buffer_iterator& operator=(
            basic_text_buffer_iterator const&) noexcept = default;

        constexpr basic_text_buffer_iterator& operator=(
            basic_text_buffer_iterator&&) noexcept = default;

        constexpr friend bool operator==(basic_text_buffer_iterator const& lhs,
            basic_text_buffer_iterator const& rhs) noexcept
        {
            return lhs.node_index_ == rhs.node_index_ &&
                lhs.local_index_ == rhs.local_index_;
        }

        constexpr friend bool operator!=(basic_text_buffer_iterator const& lhs,
            basic_text_buffer_iterator const& rhs) noexcept
        {
            return !(lhs == rhs);
        }

    private: // Helpers
        [[nodiscard]] constexpr detail::node const&
        current_node() const noexcept
        {
            return nodes_[node_index_];
        }

    private: // Data
        std::span<detail::node const> nodes_;
        size_t node_index_{};
        detail::buffer<CharT, Traits, Allocator>* buffers_{};
        size_t local_index_{};
    };

    using text_buffer = basic_text_buffer<char>;
    using wtext_wbuffer = basic_text_buffer<wchar_t>;
    using u8text_buffer = basic_text_buffer<char8_t>;
    using u16text_buffer = basic_text_buffer<char16_t>;
    using u32text_buffer = basic_text_buffer<char32_t>;

    static_assert(std::bidirectional_iterator<text_buffer::const_iterator>);
    static_assert(std::bidirectional_iterator<text_buffer::iterator>);
} // namespace afv::buf
