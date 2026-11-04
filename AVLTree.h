#ifndef AVLTREE_H
#define AVLTREE_H

#include <algorithm>
#include <utility>
#include <vector>

template <typename K, typename V>
class AVLTree {
private:
    struct Node {
        K key;
        V value;
        Node* left;
        Node* right;
        int height;  // a leaf has height 1, an empty subtree (nullptr) has height 0

        Node(const K& k, const V& v) : key(k), value(v), left(nullptr), right(nullptr), height(1) {}
    };

    Node* root;
    int count;

    // ---------- small helpers ----------

    static int height(const Node* n) { return n ? n->height : 0; }

    // BF(N) = height(left) - height(right). >1 means left-heavy, <-1 means right-heavy.
    static int balanceFactor(const Node* n) { return n ? height(n->left) - height(n->right) : 0; }

    static void updateHeight(Node* n) { n->height = 1 + std::max(height(n->left), height(n->right)); }

    // ---------- rotations ----------
    // Each rotation returns the new root of the subtree so the caller can relink it.
    // Heights are updated bottom-up: the node that moved down first, then the new root.

    /* Left-Left case: y's left child x becomes the subtree root.
     *
     *         y                x
     *        / \              / \
     *       x   C    -->     A   y
     *      / \                  / \
     *     A   B                B   C
     */
    static Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* B = x->right;
        x->right = y;
        y->left = B;
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    /* Right-Right case: mirror image of rotateRight.
     *
     *       x                    y
     *      / \                  / \
     *     A   y      -->       x   C
     *        / \              / \
     *       B   C            A   B
     */
    static Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* B = y->left;
        y->left = x;
        x->right = B;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    // Left-Right case: the left child leans right. Straighten it with a left rotation,
    // which turns it into a Left-Left case, then rotate right.
    static Node* rotateLeftRight(Node* n) {
        n->left = rotateLeft(n->left);
        return rotateRight(n);
    }

    // Right-Left case: mirror image of rotateLeftRight.
    static Node* rotateRightLeft(Node* n) {
        n->right = rotateRight(n->right);
        return rotateLeft(n);
    }

    // Recomputes n's height and fixes an imbalance at n, if there is one.
    // Returns the (possibly new) root of this subtree.
    static Node* rebalance(Node* n) {
        updateHeight(n);
        int bf = balanceFactor(n);

        if (bf > 1) {                              // left-heavy
            if (balanceFactor(n->left) >= 0)
                return rotateRight(n);             // Left-Left
            return rotateLeftRight(n);             // Left-Right
        }
        if (bf < -1) {                             // right-heavy
            if (balanceFactor(n->right) <= 0)
                return rotateLeft(n);              // Right-Right
            return rotateRightLeft(n);             // Right-Left
        }
        return n;                                  // already balanced
    }

    // ---------- recursive workers ----------

    // Inserts below n and returns the new root of this subtree. Balance is restored
    // as the recursion unwinds, which is what the spec means by "on recursion unwind".
    Node* insert(Node* n, const K& key, const V& value) {
        if (!n) {
            ++count;
            return new Node(key, value);
        }
        if (key < n->key)
            n->left = insert(n->left, key, value);
        else if (n->key < key)
            n->right = insert(n->right, key, value);
        else {
            n->value = value;                      // duplicate key: overwrite, nothing changes shape
            return n;
        }
        return rebalance(n);
    }

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    // Debug check: returns the subtree's height, or -1 if anything is wrong
    // (BST order broken, |BF| > 1, or a stored height that doesn't match reality).
    static int validate(const Node* n, const K* lo, const K* hi) {
        if (!n) return 0;
        if ((lo && !(*lo < n->key)) || (hi && !(n->key < *hi))) return -1;
        int hl = validate(n->left, lo, &n->key);
        int hr = validate(n->right, &n->key, hi);
        if (hl < 0 || hr < 0) return -1;
        if (hl - hr > 1 || hr - hl > 1) return -1;
        int h = 1 + std::max(hl, hr);
        return h == n->height ? h : -1;
    }

public:
    AVLTree() : root(nullptr), count(0) {}

    ~AVLTree() { destroy(root); }

    // Copying is disabled until the Rule of Three deep copy is written (day 8).
    // Without this, a copy would share nodes and both destructors would free them.
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    void insert(const K& key, const V& value) { root = insert(root, key, value); }

    // Iterative: walks one root-to-leaf path, so O(log n) in a balanced tree.
    bool search(const K& key, V& outValue) const {
        const Node* cur = root;
        while (cur) {
            if (key < cur->key)
                cur = cur->left;
            else if (cur->key < key)
                cur = cur->right;
            else {
                outValue = cur->value;
                return true;
            }
        }
        return false;
    }

    int size() const { return count; }
    bool empty() const { return count == 0; }
    int treeHeight() const { return height(root); }

    // For testing only: true if every node satisfies the BST and AVL invariants.
    bool isValid() const { return validate(root, nullptr, nullptr) >= 0; }
};

#endif
