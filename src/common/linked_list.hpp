#pragma once

#include "common/log.hpp"
#include "common/type_aliases.hpp"

namespace hydra {

template <typename T, bool is_doubly_linked = false>
class LinkedListNode {
    template <typename T_, bool is_doubly_linked_>
    friend class LinkedList;

  public:
    LinkedListNode(const T& value_) : value{value_}, next{nullptr} {}

    operator const T&() const { return value; }
    const T* operator->() const { return &value; }
    T* operator->() { return &value; }

  private:
    T value;
    LinkedListNode* next;

  public:
    CONST_REF_GETTER(value, Get);
    GETTER(next, GetNext);
};

template <typename T>
class LinkedListNode<T, true> {
    template <typename T_, bool is_doubly_linked_>
    friend class LinkedList;

  public:
    LinkedListNode(const T& value_)
        : value{value_}, next{nullptr}, prev{nullptr} {}

    operator const T&() const { return value; }
    const T* operator->() const { return &value; }
    T* operator->() { return &value; }

  private:
    T value;
    LinkedListNode* next;
    LinkedListNode* prev;

  public:
    CONST_REF_GETTER(value, Get);
    GETTER(next, GetNext);
    GETTER(prev, GetPrev);
};

template <typename T, bool is_doubly_linked>
class LinkedList {
  public:
    LinkedList() : head{nullptr}, tail{nullptr}, size{0} {}

    void AddFirst(const T& value) {
        auto node = new LinkedListNode<T, is_doubly_linked>(value);
        if (!head) {
            head = tail = node;
        } else {
            node->next = head;
            if constexpr (is_doubly_linked)
                head->prev = node;
            head = node;
        }
        size++;
    }

    void AddLast(const T& value) {
        auto node = new LinkedListNode<T, is_doubly_linked>(value);
        if (!head) {
            head = tail = node;
        } else {
            tail->next = node;
            if constexpr (is_doubly_linked)
                node->prev = tail;
            tail = node;
        }
        size++;
    }

    void RemoveFirst() {
        ASSERT_DEBUG(head, Common, "List is empty");

        auto node = head;
        head = head->next;
        delete node;
        if (!head)
            tail = nullptr;
        size--;
    }

    bool RemoveLast() {
        ASSERT_DEBUG(head, Common, "List is empty");

        if (!head->next) {
            delete head;
            head = tail = nullptr;
        } else {
            auto node = head;
            while (node->next != tail)
                node = node->next;
            delete tail;
            tail = node;
            tail->next = nullptr;
        }
        size--;
    }

    LinkedListNode<T, is_doubly_linked>*
    Remove(LinkedListNode<T, is_doubly_linked>* target) {
        ASSERT_DEBUG(target, Common, "Invalid node");
        ASSERT_DEBUG(head, Common, "List is empty");

        if constexpr (is_doubly_linked) {
            // A more efficient way to remove a node from a doubly linked list

            // Head
            if (target == head) {
                head = target->next;
            } else {
                ASSERT_DEBUG(target->prev, Common, "Invalid node");
                target->prev->next = target->next;
            }

            // Tail
            if (target == tail) {
                tail = target->prev;
            } else {
                ASSERT_DEBUG(target->next, Common, "Invalid node");
                target->next->prev = target->prev;
            }

            auto next = target->next;
            delete target;
            size--;
            return next;
        } else {
            if (head == target) {
                RemoveFirst();
                return head;
            }

            if (tail == target) {
                RemoveLast();
                return nullptr;
            }

            auto node = head;
            while (node->next && node->next != target)
                node = node->next;
            ASSERT_DEBUG(node->next, Common, "Invalid node");

            node->next = target->next;
            delete target;
            if (!node->next)
                tail = node;
            size--;
            return node->next;
        }
    }

    typename std::enable_if<is_doubly_linked,
                            LinkedListNode<T, is_doubly_linked>*>::type
    Remove(const T& target) {
        ASSERT_DEBUG(target, Common, "Invalid node");
        ASSERT_DEBUG(head, Common, "List is empty");

        // Find the node to remove
        auto node = head;
        while (node && node->value != target)
            node = node->next;
        ASSERT_DEBUG(node, Common, "Invalid node");

        // Remove the node
        return Remove(node);
    }

    void Clear() {
        // TODO: do more efficiently
        while (head)
            RemoveFirst();
    }

  private:
    LinkedListNode<T, is_doubly_linked>* head;
    LinkedListNode<T, is_doubly_linked>* tail;
    usize size;

  public:
    GETTER(head, GetHead);
    GETTER(tail, GetTail);
    GETTER(size, GetSize);
};

template <typename T>
using SingleLinkedList = LinkedList<T, false>;
template <typename T>
using DoubleLinkedList = LinkedList<T, true>;

} // namespace hydra
