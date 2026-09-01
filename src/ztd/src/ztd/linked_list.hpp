#pragma once

#include "ztd/mem/default_allocator.hpp"
#include "ztd/type_aliases.hpp"

namespace ztd {

template <typename T, bool is_doubly_linked>
class LinkedList {
  public:
    class SinglyNode {
        template <typename T_, bool is_doubly_linked_>
        friend class LinkedList;

      public:
        SinglyNode(T value_) noexcept : value{std::move(value_)} {}

        operator T&() noexcept { return value; }
        operator const T&() const noexcept { return value; }
        auto operator->() const noexcept -> const T* { return &value; }
        auto operator->() noexcept -> T* { return &value; }

        auto get() noexcept -> T& { return value; }
        auto get() const noexcept -> const T& { return value; }

        auto getNext() const noexcept -> std::optional<SinglyNode*> {
            return next;
        }

      private:
        T value;
        std::optional<SinglyNode*> next{};
    };

    class DoublyNode {
        template <typename T_, bool is_doubly_linked_>
        friend class LinkedList;

      public:
        DoublyNode(T value_) noexcept : value{std::move(value_)} {}

        operator T&() noexcept { return value; }
        operator const T&() const noexcept { return value; }
        auto operator->() const noexcept -> const T* { return &value; }
        auto operator->() noexcept -> T* { return &value; }

        auto get() noexcept -> T& { return value; }
        auto get() const noexcept -> const T& { return value; }

        auto getNext() const noexcept -> std::optional<DoublyNode*> {
            return next;
        }

        auto getPrev() const noexcept -> std::optional<DoublyNode*> {
            return prev;
        }

      private:
        T value;
        std::optional<DoublyNode*> next{};
        std::optional<DoublyNode*> prev{};
    };

    using Node =
        typename std::conditional_t<is_doubly_linked, DoublyNode, SinglyNode>;

    LinkedList(
        mem::IAllocator& allocator_ = mem::getDefaultAllocator()) noexcept
        : allocator{allocator_} {}
    ~LinkedList() noexcept { clear(); }

    ZTD_MAKE_NON_COPYABLE(LinkedList);
    ZTD_MAKE_DEFAULT_MOVABLE(LinkedList);

    auto addFirst(T value) noexcept
        -> std::expected<Node*, mem::IAllocator::Error> {
        ZTD_ASSIGN_OR_RETURN_ERROR(auto node,
                                   allocator.create<Node>(std::move(value)));
        if (head.has_value()) {
            const auto head_ = head.value();
            node->next = head_;
            if constexpr (is_doubly_linked)
                head_->prev = node;
            head = node;
        } else {
            head = tail = node;
        }
        size++;

        return node;
    }

    auto addLast(T value) noexcept
        -> std::expected<Node*, mem::IAllocator::Error> {
        ZTD_ASSIGN_OR_RETURN_ERROR(auto node,
                                   allocator.create<Node>(std::move(value)));
        if (tail.has_value()) {
            const auto tail_ = tail.value();
            tail_->next = node;
            if constexpr (is_doubly_linked)
                node->prev = tail_;
            tail = node;
        } else {
            head = tail = node;
        }
        size++;

        return node;
    }

    auto removeFirst() noexcept -> bool {
        ZTD_ASSIGN_OR_RETURN_VALUE(auto head_, head, false);

        auto node = head_;
        head = head_->next;
        allocator.destroy(node);
        if (!head.has_value())
            tail = std::nullopt;
        size--;

        return true;
    }

    auto removeLast() noexcept -> bool
        requires is_doubly_linked
    {
        ZTD_ASSIGN_OR_RETURN_VALUE(auto head_, head, false);

        if (!head_->next.has_value()) {
            allocator.destroy(head_);
            head = tail = std::nullopt;
        } else {
            auto old_tail = tail.value();
            tail = old_tail->prev;
            tail.value()->next = std::nullopt;
            allocator.destroy(old_tail);
        }
        size--;

        return true;
    }

    auto remove(Node* target) noexcept -> std::optional<Node*>
        requires is_doubly_linked
    {
        ZTD_ASSIGN_OR_RETURN_VALUE(auto head_, head, std::nullopt);
        const auto tail_ = tail.value();

        // Head
        if (target == head_) {
            head = target->next;
        } else {
            target->prev.value()->next = target->next;
        }

        // Tail
        if (target == tail_) {
            tail = target->prev;
        } else {
            target->next.value()->prev = target->prev;
        }

        auto next = target->next;
        allocator.destroy(target);
        size--;
        return next;
    }

    auto remove(const T& target) noexcept -> void {
        // Remove all occurrences of the target
        for (auto node = head; node.has_value();) {
            const auto node_ = node.value();
            if (node_->value == target) {
                if constexpr (is_doubly_linked) {
                    node = remove(node_);
                } else {
                    // TODO
                    static_assert(false, "NOT IMPLEMENTED");
                }
            } else {
                node = node_->next;
            }
        }
    }

    auto clear() noexcept -> void {
        auto node = head;
        while (node.has_value()) {
            const auto node_ = node.value();
            const auto next_node = node_->next;
            allocator.destroy(node_);
            node = next_node;
        }
        head = std::nullopt;
        tail = std::nullopt;
        size = 0;
    }

    [[nodiscard]] auto getHead() const noexcept -> std::optional<Node*> {
        return head;
    }
    [[nodiscard]] auto getTail() const noexcept -> std::optional<Node*> {
        return tail;
    }
    [[nodiscard]] auto getSize() const noexcept -> usize { return size; }

  private:
    mem::IAllocator& allocator;
    std::optional<Node*> head{};
    std::optional<Node*> tail{};
    usize size{0};
};

template <typename T>
using SinglyLinkedList = LinkedList<T, false>;

template <typename T>
using DoublyLinkedList = LinkedList<T, true>;

} // namespace ztd
