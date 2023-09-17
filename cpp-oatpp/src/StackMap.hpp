#ifndef stackmap_hpp
#define stackmap_hpp

#include <atomic>
#include <exception>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>

class StackEmpty : public std::exception {
public:
    const char *what() const noexcept override { return "Stack is empty"; }
};

class StackNameAlreadyExists : public std::exception {
public:
    const char *what() const noexcept override {
        return "Stack name already exists";
    }
};

class StackNameNotFound : public std::exception {
public:
    const char *what() const noexcept override {
        return "Stack name not found";
    }
};

template <typename T> class Stack {
public:
    Stack() : head(nullptr) {}
    ~Stack() { this->destroyLink(this->head); }
    Stack(const Stack &stack) : head(stack.copyHead()) {}
    Stack(Stack &&stack) : head(stack.head) { stack.head = nullptr; }
    Stack &operator=(const Stack &stack) noexcept {
        Node *oldHead, *newHead = stack.copyHead();
        {
            std::unique_lock _lock(stack.lock);
            oldHead = this->head;
            this->head = newHead;
        }
        this->destroyLink(oldHead);
        return *this;
    }
    Stack &operator=(Stack &&stack) noexcept {
        Node *oldHead, *newHead = stack.head;
        stack.head = nullptr;
        {
            std::unique_lock _lock(stack.lock);
            oldHead = this->head;
            this->head = newHead;
        }
        this->destroyLink(oldHead);
        return *this;
    }

    T getTop() const {
        std::shared_lock _lock(this->lock);
        if (this->head == nullptr) {
            throw StackEmpty();
        }
        return this->head->value;
    }
    void push(T &&value) {
        std::unique_lock _lock(this->lock);
        this->head = new Node(std::move(value), this->head);
    }
    T pop() {
        std::unique_lock _lock(this->lock);
        auto poppedNode = this->head;
        if (poppedNode == nullptr) {
            throw StackEmpty();
        }

        this->head = poppedNode->next;
        if (Node::unique(poppedNode)) {
            // The popped node has only one reference from this stack,
            // we can move the value out, and we don't need to modify the refernce counter of
            // current head, as it just transferred from the next of the popped node to the stack.
            auto result = std::move(poppedNode->value);
            delete poppedNode;
            return result;
        } else {
            if (this->head != nullptr) Node::incRef(this->head);
            auto result = poppedNode->value;
            
            // This reference counter decrement must be done after the operations above,
            // otherwise the popped node and the current head may be deleted.
            if (Node::decRef(poppedNode)) {
                delete poppedNode;
                if (this->head != nullptr) Node::decRef(this->head); // Must be false since the counter just increases.
            }
            return result;
        }
    }

private:
    class Node {
    public:
        Node(T &&value, Node *next)
            : refcount(1), value(std::move(value)), next(next) {}

        // https://www.boost.org/doc/libs/1_55_0/doc/html/atomic/usage_examples.html#boost_atomic.usage_examples.example_reference_counters
        static void incRef(Node *node) {
            node->refcount.fetch_add(1, std::memory_order_relaxed);
        }
        // Returns whether its reference counter is 1 before decrement.
        static bool decRef(Node *node) {
            if (node->refcount.fetch_sub(1, std::memory_order_release) == 1) {
                std::atomic_thread_fence(std::memory_order_acquire);
                return true;
            } else {
                return false;
            }
        }
        // Returns whether its reference counter is 1.
        static bool unique(Node *node) {
            return node->refcount.load(std::memory_order_acquire) == 1;
        }

        Node *next;
        T value;

    private:
        std::atomic<int> refcount;
    };

    static void destroyLink(Node *head) {
        auto ptr = head;
        while (ptr != nullptr) {
            if (!Node::decRef(ptr)) {
                // Still has other reference
                break;
            }
            auto next = ptr->next;
            delete ptr;
            ptr = next;
        }
    }

    Node *copyHead() const {
        std::shared_lock _lock(this->lock);
        auto head = this->head;
        if (head != nullptr) {
            Node::incRef(head);
        }
        return head;
    }

    Node *head;
    mutable std::shared_mutex lock;
};

template <typename K, typename T> class StackMap {
public:
    void create(K &&name) {
        bool inserted;
        {
            std::unique_lock _lock(this->lock);
            inserted = this->map.insert({std::move(name), Stack<T>()}).second;
        }
        if (!inserted) {
            throw StackNameAlreadyExists();
        }
    }

    void remove(const K &name) {
        bool removed;
        {
            std::unique_lock _lock(this->lock);
            removed = this->map.erase(name) > 0;
        }
        if (!removed) {
            throw StackNameNotFound();
        }
    }

    std::pair<std::shared_lock<std::shared_mutex>, Stack<T> &>
    getStack(const K &name) {
        std::shared_lock<std::shared_mutex> lock(this->lock);
        auto stack = this->map.find(name);
        if (stack == this->map.cend()) {
            throw StackNameNotFound();
        }
        return {std::move(lock), stack->second};
    }

    void copy(const K &from, K &&to) {
        bool inserted;
        {
            std::unique_lock _lock(this->lock);

            auto fromIter = this->map.find(from);
            if (fromIter == this->map.cend()) {
                throw StackNameNotFound();
            }

            inserted =
                this->map.insert({std::move(to), fromIter->second}).second;
        }

        if (!inserted) {
            throw StackNameAlreadyExists();
        }
    }

private:
    std::shared_mutex lock;
    std::unordered_map<K, Stack<T>> map;
};

#endif
