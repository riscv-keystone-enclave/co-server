#pragma once
#include <atomic>
#include <memory>
#include <utility>

template <typename T>
class LockFreeQueue
{
private:
    struct Node
    {
        T data;
        std::atomic<Node *> next;

        Node() : next(nullptr) {}

        Node(const T &value) : data(value), next(nullptr) {}
        Node(T &&value) : data(std::move(value)), next(nullptr) {}
    };

    std::atomic<Node *> head;
    std::atomic<Node *> tail;
    std::atomic<ssize_t> size;
    std::atomic<ssize_t> capacity;

private:
    void _enqueue(Node *newNode)
    {
        Node *last = tail.load();
        Node *nullNode = nullptr;
        while (true)
        {
            Node *next = last->next.load();
            if (!next)
            {
                if (last->next.compare_exchange_strong(nullNode, newNode))
                {
                    tail.compare_exchange_strong(last, newNode);
                    size.fetch_add(1);
                    return;
                }
            }
            else
            {
                tail.compare_exchange_strong(last, next);
            }
            last = tail.load();
        }
    }

public:
    LockFreeQueue(ssize_t cap) : capacity(cap), size(0)
    {
        Node *dummy = new Node();
        head.store(dummy);
        tail.store(dummy);
    }

    ~LockFreeQueue()
    {
        T d;
        while (dequeue(d))
        {
        }
        Node *dummy = head.load();
        delete dummy;
    }

    bool enqueue(const T &value)
    {
        if (size >= capacity)
        {
            return false;
        }
        Node *newNode = new Node(value);
        _enqueue(newNode);
        return true;
    }

    bool enqueue(T &&value)
    {
        if (size >= capacity)
        {
            return false;
        }
        Node *newNode = new Node(std::move(value));
        _enqueue(newNode);
        return true;
    }

    bool dequeue(T &value)
    {
        if(size <= 0) {
            return false;
        }

        Node *nullNode = nullptr;
        while (true)
        {
            Node *first = head.load();
            Node *last = tail.load();
            Node *next = first->next.load();
            if (first == head.load())
            {
                if (first == last)
                {
                    if (!next)
                    {
                        return false;
                    }
                    tail.compare_exchange_strong(last, next);
                }
                else
                {
                    value = std::move(next->data);
                    if (head.compare_exchange_strong(first, next))
                    {
                        delete first;
                        size.fetch_sub(1);
                        return true;
                    }
                }
            }
        }
    }
};

