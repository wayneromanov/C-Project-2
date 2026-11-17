// Tests for AVLTree. Not part of the submission.
// Build and run: make test_avl && ./test_avl   (or `make test` for every test)

#include <algorithm>
#include <cassert>
#include <chrono>
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

    std::cout << "remove: missing keys and empty tree\n";
    {
        AVLTree<int, int> t;
        CHECK(!t.remove(5));
        t.insert(5, 5);
        CHECK(!t.remove(4) && t.size() == 1);
        CHECK(t.remove(5) && t.empty() && t.treeHeight() == 0);
        CHECK(!t.remove(5));
    }

    std::cout << "remove: leaf, one child, two children, root\n";
    {
        /*          4
         *        /   \
         *       2     6
         *      / \   / \
         *     1   3 5   7
         *                \
         *                 8
         */
        AVLTree<int, int> t;
        for (int k : {4, 2, 6, 1, 3, 5, 7, 8}) t.insert(k, k);
        CHECK(t.isValid());
        int v = 0;

        CHECK(t.remove(1) && t.isValid() && !t.search(1, v));      // leaf
        CHECK(t.remove(7) && t.isValid() && !t.search(7, v));      // one child (8 moves up)
        CHECK(t.search(8, v) && v == 8);
        CHECK(t.remove(6) && t.isValid() && !t.search(6, v));      // two children (successor 8)
        CHECK(t.remove(4) && t.isValid() && !t.search(4, v));      // the root
        CHECK(t.size() == 4);
        for (int k : {2, 3, 5, 8}) CHECK(t.search(k, v) && v == k); // values moved with keys
    }

    // Deleting can leave the taller child perfectly balanced (BF 0), which never happens
    // after an insert. A single rotation is the correct fix in that case.
    std::cout << "remove: rotations, including the BF-0 child case\n";
    {
        AVLTree<int, int> t;                     // 2 with right child 4 (children 3 and 5)
        for (int k : {2, 1, 4, 3, 5}) t.insert(k, k);
        CHECK(t.remove(1) && t.isValid() && t.treeHeight() == 3);

        AVLTree<int, int> m;                     // mirror image
        for (int k : {4, 5, 2, 1, 3}) m.insert(k, k);
        CHECK(m.remove(5) && m.isValid() && m.treeHeight() == 3);

        AVLTree<int, int> lr;                    // deleting 4 leaves a Left-Right imbalance at 3
        for (int k : {3, 1, 4, 2}) lr.insert(k, k);
        CHECK(lr.remove(4) && lr.isValid() && lr.treeHeight() == 2);

        AVLTree<int, int> rl;                    // Right-Left imbalance at 2
        for (int k : {2, 1, 4, 3}) rl.insert(k, k);
        CHECK(rl.remove(1) && rl.isValid() && rl.treeHeight() == 2);
    }

    // Random inserts and removes, compared after every step against a sorted vector
    // that stands in for the "right answer".
    std::cout << "random insert/remove vs. reference, checked after every step\n";
    {
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> keyDist(0, 499), op(0, 2);
        AVLTree<int, int> t;
        std::vector<int> ref;                    // sorted keys currently in the tree
        for (int step = 0; step < 20000; ++step) {
            int k = keyDist(rng);
            auto it = std::lower_bound(ref.begin(), ref.end(), k);
            bool present = it != ref.end() && *it == k;
            if (op(rng) == 0) {                  // 1/3 removes, 2/3 inserts
                CHECK(t.remove(k) == present);
                if (present) ref.erase(it);
            } else {
                t.insert(k, k + step);
                if (!present) ref.insert(it, k);
            }
            CHECK(t.isValid() && t.size() == (int)ref.size());
        }
        for (int k = 0; k < 500; ++k) {
            int v;
            CHECK(t.search(k, v) == std::binary_search(ref.begin(), ref.end(), k));
        }
        for (int k : std::vector<int>(ref)) CHECK(t.remove(k) && t.isValid());
        CHECK(t.empty() && t.treeHeight() == 0);
    }

    std::cout << "remove with string keys\n";
    {
        AVLTree<std::string, int> t;
        for (std::string k : {"m", "c", "x", "a", "e", "z", "d"}) t.insert(k, (int)k[0]);
        CHECK(t.remove("c") && t.isValid());     // two children: successor "d"
        int v = 0;
        CHECK(t.search("d", v) && v == 'd' && !t.search("c", v));
    }

    std::cout << "rangeQuery matches a brute-force filter\n";
    {
        std::mt19937 rng(7);
        AVLTree<int, int> t;
        std::vector<int> ref;
        for (int i = 0; i < 300; ++i) {
            int k = (int)(rng() % 1000);
            if (!std::binary_search(ref.begin(), ref.end(), k))
                ref.insert(std::lower_bound(ref.begin(), ref.end(), k), k);
            t.insert(k, k * 3);
        }
        for (int q = 0; q < 2000; ++q) {
            int lo = (int)(rng() % 1100) - 50, hi = (int)(rng() % 1100) - 50;
            std::vector<std::pair<int, int>> got = t.rangeQuery(lo, hi);
            std::vector<std::pair<int, int>> want;
            for (int k : ref)
                if (lo <= k && k <= hi) want.push_back({k, k * 3});
            CHECK(got == want);                  // same entries, same ascending order
        }
        AVLTree<int, int> empty;
        CHECK(empty.rangeQuery(0, 100).empty());
        CHECK(t.rangeQuery(ref.front(), ref.front()).size() == 1);   // low == high, inclusive
    }

    // Pruning check: with 200,000 nodes, 20,000 narrow queries take milliseconds if subtrees
    // are pruned, but would visit 4 billion nodes (many seconds) with a full traversal.
    std::cout << "rangeQuery prunes (timing)\n";
    {
        AVLTree<int, int> t;
        for (int i = 0; i < 200000; ++i) t.insert(i, i);
        auto start = std::chrono::steady_clock::now();
        long long found = 0;
        for (int q = 0; q < 20000; ++q) found += (long long)t.rangeQuery(q * 10, q * 10 + 4).size();
        double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        CHECK(found == 20000LL * 5);
        CHECK(secs < 1.0);
        std::cout << "  20000 queries in " << secs * 1000 << " ms\n";
    }

    std::cout << "copy constructor and assignment are deep\n";
    {
        AVLTree<std::string, int> a;
        for (int i = 0; i < 100; ++i) a.insert("k" + std::to_string(i), i);

        AVLTree<std::string, int> b(a);          // copy constructor
        CHECK(b.isValid() && b.size() == 100 && b.treeHeight() == a.treeHeight());
        b.insert("k5", 500);
        b.remove("k6");
        int v = 0;
        CHECK(a.search("k5", v) && v == 5);      // changing the copy leaves the original alone
        CHECK(a.search("k6", v) && a.size() == 100);

        AVLTree<std::string, int> c;
        c.insert("old", 1);                      // these nodes must be freed by operator=
        c = a;
        CHECK(c.isValid() && c.size() == 100 && !c.search("old", v));
        a.remove("k1");
        CHECK(c.search("k1", v) && v == 1);

        AVLTree<std::string, int>& alias = c;    // (written via a reference so the compiler
        c = alias;                               // doesn't warn) self-assignment must not free its own nodes
        CHECK(c.isValid() && c.size() == 100);

        AVLTree<std::string, int> e;
        c = e;                                   // assigning an empty tree
        CHECK(c.empty() && c.isValid());
        AVLTree<std::string, int> d(e);          // copying an empty tree
        CHECK(d.empty());
    }

    std::cout << "forEach visits every entry in key order\n";
    {
        AVLTree<int, int> t;
        for (int k : {5, 3, 8, 1, 4, 9}) t.insert(k, k);
        std::vector<int> seen;
        t.forEach([&](int k, int) { seen.push_back(k); });
        CHECK((seen == std::vector<int>{1, 3, 4, 5, 8, 9}));
    }

    std::cout << "\nall " << passed << " checks passed\n";
    return 0;
}
