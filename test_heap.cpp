// Tests for MinHeap. Not part of the submission.
// Build and run: make test_heap && ./test_heap   (or `make test` for every test)

#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "MinHeap.h"

static int passed = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::cerr << "FAIL line " << __LINE__ << ": " #cond << std::endl;    \
            return 1;                                                            \
        }                                                                        \
        ++passed;                                                                \
    } while (0)

int main() {
    std::cout << "empty heap\n";
    {
        MinHeap<int> h;
        CHECK(h.empty() && h.size() == 0);
    }

    std::cout << "single element\n";
    {
        MinHeap<int> h;
        h.push(7);
        CHECK(!h.empty() && h.size() == 1 && h.peek() == 7);
        CHECK(h.pop() == 7 && h.empty());
    }

    std::cout << "pops come out sorted (random input, duplicates)\n";
    {
        std::mt19937 rng(2110);
        for (int n : {2, 3, 10, 1000}) {
            std::vector<int> xs(n);
            for (int& x : xs) x = (int)(rng() % 50);
            MinHeap<int> h;
            for (int x : xs) {
                h.push(x);
                CHECK(h.peek() == *std::min_element(xs.begin(), xs.begin() + (long)h.size()));
            }
            std::sort(xs.begin(), xs.end());
            for (int x : xs) CHECK(h.pop() == x);
            CHECK(h.empty());
        }
    }

    std::cout << "sorted and reverse-sorted input\n";
    {
        MinHeap<int> up, down;
        for (int i = 0; i < 500; ++i) {
            up.push(i);
            down.push(499 - i);
        }
        for (int i = 0; i < 500; ++i) CHECK(up.pop() == i && down.pop() == i);
    }

    std::cout << "interleaved push and pop\n";
    {
        std::mt19937 rng(5);
        MinHeap<int> h;
        std::vector<int> ref;                    // kept sorted
        for (int step = 0; step < 5000; ++step) {
            if (!ref.empty() && rng() % 3 == 0) {
                CHECK(h.pop() == ref.front());
                ref.erase(ref.begin());
            } else {
                int x = (int)(rng() % 1000);
                h.push(x);
                ref.insert(std::upper_bound(ref.begin(), ref.end(), x), x);
            }
            CHECK(h.size() == ref.size());
            if (!ref.empty()) CHECK(h.peek() == ref.front());
        }
    }

    std::cout << "works with other types (strings)\n";
    {
        MinHeap<std::string> h;
        for (std::string s : {"pear", "apple", "fig", "banana"}) h.push(s);
        CHECK(h.pop() == "apple" && h.pop() == "banana" && h.pop() == "fig" && h.pop() == "pear");
    }

    std::cout << "\nall " << passed << " checks passed\n";
    return 0;
}
