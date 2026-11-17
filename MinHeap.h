#ifndef MINHEAP_H
#define MINHEAP_H

#include <algorithm>
#include <cassert>
#include <vector>

// Array-backed binary min-heap: the smallest element (by operator<) is always at index 0.
// The tree shape is implicit in the array indices:
//   parent(i) = (i - 1) / 2,  left(i) = 2i + 1,  right(i) = 2i + 2
template <typename T>
class MinHeap {
private:
    std::vector<T> data;

    static std::size_t parent(std::size_t i) { return (i - 1) / 2; }
    static std::size_t left(std::size_t i) { return 2 * i + 1; }
    static std::size_t right(std::size_t i) { return 2 * i + 2; }

    // Moves the element at i up while it is smaller than its parent.
    void siftUp(std::size_t i) {
        while (i > 0 && data[i] < data[parent(i)]) {
            std::swap(data[i], data[parent(i)]);
            i = parent(i);
        }
    }

    // Moves the element at i down, swapping with its smaller child, until neither child is smaller.
    void siftDown(std::size_t i) {
        std::size_t n = data.size();
        while (true) {
            std::size_t smallest = i;
            std::size_t l = left(i), r = right(i);
            if (l < n && data[l] < data[smallest]) smallest = l;
            if (r < n && data[r] < data[smallest]) smallest = r;
            if (smallest == i) return;
            std::swap(data[i], data[smallest]);
            i = smallest;
        }
    }

public:
    // O(log n): append at the end (the next free leaf), then sift up.
    void push(const T& element) {
        data.push_back(element);
        siftUp(data.size() - 1);
    }

    // O(log n): take the root, move the last leaf into its place, then sift down.
    T pop() {
        assert(!data.empty());
        T top = data[0];
        data[0] = data.back();
        data.pop_back();
        if (!data.empty()) siftDown(0);
        return top;
    }

    // O(1)
    const T& peek() const {
        assert(!data.empty());
        return data[0];
    }

    std::size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
};

#endif
