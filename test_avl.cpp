// Tests for AVL insert, rotations and search (days 1-2). Not part of the submission.
// Build: g++ -std=c++17 -Wall -Wextra -Werror -g test_avl.cpp -o test_avl && ./test_avl

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "AVLTree.h"

static int passed = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::cerr << "FAIL line " << __LINE__ << ": " #cond << std::endl;    \
            return 1;                                                            \
        }                                                                        \
        ++passed;                                                                \
    } while (0)

// Each three-key order triggers exactly one rotation type; afterwards the middle key
// must be the root, so the tree has height 2.
static int testRotation(const std::string& name, int a, int b, int c) {
    AVLTree<int, int> t;
    t.insert(a, a);
    t.insert(b, b);
    t.insert(c, c);
    CHECK(t.isValid());
    CHECK(t.treeHeight() == 2);
    for (int k : {a, b, c}) {
        int v = -1;
        CHECK(t.search(k, v) && v == k);
    }
    std::cout << "  " << name << " ok\n";
    return 0;
}

int main() {
    std::cout << "rotations\n";
    if (testRotation("Left-Left   (3,2,1)", 3, 2, 1)) return 1;
    if (testRotation("Right-Right (1,2,3)", 1, 2, 3)) return 1;
    if (testRotation("Left-Right  (3,1,2)", 3, 1, 2)) return 1;
    if (testRotation("Right-Left  (1,3,2)", 1, 3, 2)) return 1;

    std::cout << "empty tree\n";
    {
        AVLTree<std::string, int> t;
        int v = 42;
        CHECK(!t.search("x", v));
        CHECK(v == 42);                      // outValue untouched on a miss
        CHECK(t.empty() && t.isValid());
    }

    std::cout << "duplicate key overwrites\n";
    {
        AVLTree<std::string, int> t;
        t.insert("apple", 1);
        t.insert("apple", 99);
        int v = 0;
        CHECK(t.search("apple", v) && v == 99);
        CHECK(t.size() == 1);
    }

    std::cout << "string keys are lexicographic\n";
    {
        AVLTree<std::string, int> t;
        for (std::string k : {"9", "10", "100", "b", "a", "B"}) t.insert(k, 0);
        CHECK(t.isValid() && t.size() == 6);
    }

    // Sorted input is the worst case for a plain BST (it becomes a linked list).
    // An AVL tree of n nodes has height < 1.44 * log2(n + 2).
    const int N = 10000;
    std::cout << "sorted / reverse inserts (n=" << N << ")\n";
    for (int dir = 0; dir < 2; ++dir) {
        AVLTree<int, int> t;
        for (int i = 0; i < N; ++i) {
            int k = dir == 0 ? i : N - i;
            t.insert(k, k * 2);
            if (i % 997 == 0) CHECK(t.isValid());
        }
        CHECK(t.isValid() && t.size() == N);
        CHECK(t.treeHeight() <= 1.44 * std::log2(N + 2));
    }

    std::cout << "random inserts, checked after every insert\n";
    {
        std::mt19937 rng(2110);
        std::vector<int> keys(2000);
        for (int i = 0; i < (int)keys.size(); ++i) keys[i] = i;
        std::shuffle(keys.begin(), keys.end(), rng);

        AVLTree<int, int> t;
        for (int k : keys) {
            t.insert(k, -k);
            CHECK(t.isValid());
        }
        for (int k : keys) {
            int v = 0;
            CHECK(t.search(k, v) && v == -k);
        }
        int v = 0;
        CHECK(!t.search(-1, v) && !t.search(2000, v));
    }

    std::cout << "\nall " << passed << " checks passed\n";
    return 0;
}
