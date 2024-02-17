#pragma once

#include <memory>
#include <memory_resource>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace afv::buf
{
    template<typename Buffer>
    class basic_text_buffer_const_iterator;

    template<typename Buffer>
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

        using iterator = basic_text_buffer_iterator<basic_text_buffer>;
        using const_iterator =
            basic_text_buffer_const_iterator<basic_text_buffer>;

    private: // Types
        struct node
        {
            std::size_t buffer_index{};
            std::size_t start_offset{};
            std::size_t length{};
        };

        using buffer_t = std::basic_string<CharT, Traits, Allocator>;

        using buffer_allocator_t =
            std::allocator_traits<Allocator>::template rebind_alloc<buffer_t>;

        using buffer_container_t = std::vector<buffer_t, buffer_allocator_t>;

        using node_allocator_t =
            std::allocator_traits<Allocator>::template rebind_alloc<node>;

        using node_container_t = std::vector<node, node_allocator_t>;

    public: // Construction
        constexpr basic_text_buffer() noexcept(
            std::is_nothrow_constructible_v<Allocator> &&
            std::is_nothrow_constructible_v<buffer_container_t, Allocator> &&
            std::is_nothrow_constructible_v<node_container_t, Allocator>)
            : buffers_{Allocator{}}
            , nodes_{Allocator{}}
        {
        }

        constexpr explicit basic_text_buffer(Allocator const& alloc) noexcept(
            std::is_nothrow_constructible_v<buffer_container_t, Allocator> &&
            std::is_nothrow_constructible_v<node_container_t, Allocator>)
            : buffers_{alloc}
            , nodes_{alloc}
        {
        }

        constexpr basic_text_buffer(basic_text_buffer const&) noexcept(
            std::is_nothrow_copy_constructible_v<buffer_container_t> &&
            std::is_nothrow_copy_constructible_v<node_container_t>) = default;

        constexpr basic_text_buffer(basic_text_buffer&&) noexcept(
            std::is_nothrow_move_constructible_v<buffer_container_t> &&
            std::is_nothrow_move_constructible_v<node_container_t>) = default;

    public: // Destruction
        ~basic_text_buffer() = default;

    public: // Interface
        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return nodes_.empty();
        }

        template<std::ranges::range Range>
        constexpr void insert(size_type const position, Range const& range)
        {
            insert(position,
                std::ranges::begin(range),
                std::ranges::end(range));
        }

        template<std::forward_iterator Iterator,
            std::sentinel_for<Iterator> Sentinel>
        constexpr void
        insert(size_type const position, Iterator begin, Sentinel end)
        {
            size_type split_at{};
            auto const node_it{std::ranges::find_if(nodes_,
                [running_sum = size_type{}, position, &split_at](
                    node const& node) mutable noexcept
                {
                    if (position >= running_sum &&
                        position < running_sum + node.length)
                    {
                        split_at = position - running_sum;
                        return true;
                    }
                    running_sum += node.length;
                    return false;
                })};

            auto const& text{buffers_.emplace_back(begin, end)};
            if (node_it == nodes_.cend()) // Appending to end
            {
                nodes_.emplace_back(buffers_.size() - 1, 0, text.size());
                return;
            }

            if (split_at == 0) // Add buffer before the current one
            {
                nodes_.insert(node_it,
                    node{.buffer_index = buffers_.size() - 1,
                        .start_offset = 0,
                        .length = text.size()});
                return;
            }

            // Split the existing node into two and new node in the middle
            node new_node{buffers_.size() - 1, 0, text.size()};
            node new_split{node_it->buffer_index,
                split_at,
                node_it->length - split_at};

            node_it->length = split_at;
            nodes_.insert(std::next(node_it), {new_node, new_split});
        }

    public: // Iterators
        [[nodiscard]] constexpr iterator begin() noexcept
        {
            return iterator{*this};
        }

        [[nodiscard]] constexpr iterator end() noexcept
        {
            return iterator{*this, 0};
        }

        [[nodiscard]] constexpr const_iterator begin() const noexcept
        {
            return const_iterator{*this};
        }

        [[nodiscard]] constexpr const_iterator end() const noexcept
        {
            return const_iterator{*this, 0};
        }

        [[nodiscard]] constexpr const_iterator cbegin() const noexcept
        {
            return const_iterator{*this};
        }

        [[nodiscard]] constexpr const_iterator cend() const noexcept
        {
            return const_iterator{*this, 0};
        }

    public: // Operators
        constexpr basic_text_buffer&
        operator=(basic_text_buffer const&) noexcept(
            std::is_nothrow_copy_assignable_v<buffer_container_t> &&
            std::is_nothrow_copy_assignable_v<node_container_t>) = default;

        constexpr basic_text_buffer& operator=(basic_text_buffer&&) noexcept(
            std::is_nothrow_move_assignable_v<buffer_container_t> &&
            std::is_nothrow_move_assignable_v<node_container_t>) = default;

    private: // Data
        buffer_container_t buffers_;
        node_container_t nodes_;

    private:
        friend class basic_text_buffer_iterator<basic_text_buffer>;
        friend class basic_text_buffer_const_iterator<basic_text_buffer>;
    };

    using text_buffer = basic_text_buffer<char>;
    using wtext_wbuffer = basic_text_buffer<wchar_t>;
    using u8text_buffer = basic_text_buffer<char8_t>;
    using u16text_buffer = basic_text_buffer<char16_t>;
    using u32text_buffer = basic_text_buffer<char32_t>;

    template<typename CharT, typename Traits, typename Allocator>
    class basic_text_buffer_const_iterator<
        basic_text_buffer<CharT, Traits, Allocator>>
        final
    {
    public: // Types
        using value_type = std::add_const_t<
            typename basic_text_buffer<CharT, Traits, Allocator>::value_type>;
        using reference = std::add_const_t<
            typename basic_text_buffer<CharT, Traits, Allocator>::reference>;
        using difference_type =
            basic_text_buffer<CharT, Traits, Allocator>::difference_type;

    public: // Construction
        constexpr basic_text_buffer_const_iterator() = default;

        constexpr explicit basic_text_buffer_const_iterator(
            basic_text_buffer<CharT, Traits, Allocator> const& buffer)
            : nodes_{&buffer.nodes_}
            , buffers_{buffer.buffers_.data()}
        {
        }

        constexpr basic_text_buffer_const_iterator(
            basic_text_buffer<CharT, Traits, Allocator> const& buffer,
            int)
            : nodes_{&buffer.nodes_}
            , node_index_{buffer.nodes_.size()}
            , buffers_{buffer.buffers_.data()}
            , local_index_{buffer.nodes_.back().length}
        {
        }

        constexpr basic_text_buffer_const_iterator(
            basic_text_buffer_const_iterator const&) noexcept = default;

        constexpr basic_text_buffer_const_iterator(
            basic_text_buffer_const_iterator&&) noexcept = default;

    public: // Destruction
        ~basic_text_buffer_const_iterator() = default;

    public: // Operators
        constexpr value_type& operator*() const noexcept
        {
            auto const& node{current_node()};
            typename basic_text_buffer<CharT, Traits, Allocator>::
                buffer_t const& buffer{buffers_[node.buffer_index]};
            auto const value_index{node.start_offset + local_index_};
            return buffer[value_index];
        }

        constexpr value_type* operator->() const noexcept
        {
            return &(*(*this));
        }

        constexpr basic_text_buffer_const_iterator& operator++() noexcept
        {
            auto const& node{current_node()};
            if (++local_index_ == node.length &&
                ++node_index_ != nodes_->size())
            {
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
            auto const& node{current_node()};
            if (local_index_ == 0 && node_index_ > 0)
            {
                local_index_ = (*nodes_)[--node_index_].length;
                // Intentional falltrough
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
            basic_text_buffer_const_iterator const& rhs) noexcept = default;

        constexpr friend bool operator!=(
            basic_text_buffer_const_iterator const& lhs,
            basic_text_buffer_const_iterator const& rhs) noexcept = default;

    private: // Helpers
        [[nodiscard]] constexpr
            typename basic_text_buffer<CharT, Traits, Allocator>::node const&
            current_node() const noexcept
        {
            return (*nodes_)[node_index_];
        }

    private: // Data
        basic_text_buffer<CharT, Traits, Allocator>::node_container_t const*
            nodes_{};
        size_t node_index_{};
        basic_text_buffer<CharT, Traits, Allocator>::buffer_t const* buffers_{};
        size_t local_index_{};
    };

    static_assert(std::bidirectional_iterator<text_buffer::const_iterator>);

    template<typename CharT, typename Traits, typename Allocator>
    class basic_text_buffer_iterator<
        basic_text_buffer<CharT, Traits, Allocator>>
        final
    {
    public: // Types
        using value_type =
            basic_text_buffer<CharT, Traits, Allocator>::value_type;
        using reference =
            basic_text_buffer<CharT, Traits, Allocator>::reference;
        using difference_type =
            basic_text_buffer<CharT, Traits, Allocator>::difference_type;

    public: // Construction
        constexpr basic_text_buffer_iterator() = default;

        constexpr explicit basic_text_buffer_iterator(
            basic_text_buffer<CharT, Traits, Allocator>& buffer)
            : nodes_{&buffer.nodes_}
            , buffers_{buffer.buffers_.data()}
        {
        }

        constexpr basic_text_buffer_iterator(
            basic_text_buffer<CharT, Traits, Allocator>& buffer,
            int)
            : nodes_{&buffer.nodes_}
            , node_index_{buffer.nodes_.size()}
            , buffers_{buffer.buffers_.data()}
            , local_index_{buffer.nodes_.back().length}
        {
        }

        constexpr basic_text_buffer_iterator(
            basic_text_buffer_iterator const&) noexcept = default;

        constexpr basic_text_buffer_iterator(
            basic_text_buffer_iterator&&) noexcept = default;

    public: // Destruction
        ~basic_text_buffer_iterator() = default;

    public: // Operators
        constexpr value_type& operator*() const noexcept
        {
            auto const& node{current_node()};
            typename basic_text_buffer<CharT, Traits, Allocator>::buffer_t&
                buffer{buffers_[node.buffer_index]};
            auto const value_index{node.start_offset + local_index_};
            return buffer[value_index];
        }

        constexpr value_type* operator->() const noexcept
        {
            return &(*(*this));
        }

        constexpr basic_text_buffer_iterator& operator++() noexcept
        {
            auto const& node{current_node()};
            if (++local_index_ == node.length &&
                ++node_index_ != nodes_->size())
            {
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
            auto const& node{current_node()};
            if (local_index_ == 0 && node_index_ > 0)
            {
                local_index_ = (*nodes_)[--node_index_].length;
                // Intentional falltrough
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
            basic_text_buffer_iterator const& rhs) noexcept = default;

        constexpr friend bool operator!=(basic_text_buffer_iterator const& lhs,
            basic_text_buffer_iterator const& rhs) noexcept = default;

    private: // Helpers
        [[nodiscard]] constexpr
            typename basic_text_buffer<CharT, Traits, Allocator>::node const&
            current_node() const noexcept
        {
            return (*nodes_)[node_index_];
        }

    private: // Data
        basic_text_buffer<CharT, Traits, Allocator>::node_container_t* nodes_{};
        size_t node_index_{};
        basic_text_buffer<CharT, Traits, Allocator>::buffer_t* buffers_{};
        size_t local_index_{};
    };

    static_assert(std::bidirectional_iterator<text_buffer::iterator>);
} // namespace afv::buf