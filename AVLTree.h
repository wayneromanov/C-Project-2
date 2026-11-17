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

    static Node* minNode(Node* n) {
        while (n->left) n = n->left;
        return n;
    }

    // Removes key from below n and returns the new root of this subtree.
    // `removed` reports whether the key was found.
    Node* remove(Node* n, const K& key, bool& removed) {
        if (!n) return nullptr;                    // key not in this subtree
        if (key < n->key)
            n->left = remove(n->left, key, removed);
        else if (n->key < key)
            n->right = remove(n->right, key, removed);
        else {
            removed = true;
            if (!n->left || !n->right) {
                // 0 or 1 child: splice the child (possibly nullptr) into n's place.
                Node* child = n->left ? n->left : n->right;
                delete n;
                --count;
                return child;
            }
            // 2 children: copy the in-order successor (smallest key in the right subtree)
            // into n, then delete the successor's original node from the right subtree.
            // The successor has no left child, so that delete hits the 0/1-child case.
            Node* succ = minNode(n->right);
            n->key = succ->key;
            n->value = succ->value;
            n->right = remove(n->right, n->key, removed);
        }
        return rebalance(n);                       // every ancestor of the deleted node is rebalanced
    }

    // Collects keys in [low, high] in ascending (in-order) order. A subtree is only
    // entered if it can hold a key in range: everything left of n is smaller than
    // n->key, so the left side is skipped when n->key <= low, and the right side is
    // skipped when n->key >= high. That limits the work to the matches plus the two
    // boundary paths: O(k + log n) rather than visiting all n nodes.
    static void rangeQuery(const Node* n, const K& low, const K& high,
                           std::vector<std::pair<K, V>>& out) {
        if (!n) return;
        if (low < n->key)
            rangeQuery(n->left, low, high, out);
        if (!(n->key < low) && !(high < n->key))
            out.push_back(std::make_pair(n->key, n->value));
        if (n->key < high)
            rangeQuery(n->right, low, high, out);
    }

    template <typename F>
    static void inOrder(const Node* n, F& visit) {
        if (!n) return;
        inOrder(n->left, visit);
        visit(n->key, n->value);
        inOrder(n->right, visit);
    }

    // Deep copy: every node is newly allocated, so the copy shares nothing with the original.
    static Node* clone(const Node* n) {
        if (!n) return nullptr;
        Node* c = new Node(n->key, n->value);
        c->height = n->height;
        c->left = clone(n->left);
        c->right = clone(n->right);
        return c;
    }

    // Post-order: both children are freed before their parent, so no pointer is used after delete.
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

    AVLTree(const AVLTree& other) : root(clone(other.root)), count(other.count) {}

    AVLTree& operator=(const AVLTree& other) {
        if (this != &other) {                      // a = a must not free its own nodes
            Node* copy = clone(other.root);
            destroy(root);                         // release the old nodes before taking the copy
            root = copy;
            count = other.count;
        }
        return *this;
    }

    void insert(const K& key, const V& value) { root = insert(root, key, value); }

    bool remove(const K& key) {
        bool removed = false;
        root = remove(root, key, removed);
        return removed;
    }

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

    std::vector<std::pair<K, V>> rangeQuery(const K& low, const K& high) const {
        std::vector<std::pair<K, V>> out;
        rangeQuery(root, low, high, out);
        return out;
    }

    // Calls visit(key, value) for every entry in ascending key order, without building a list.
    template <typename F>
    void forEach(F visit) const { inOrder(root, visit); }

    int size() const { return count; }
    bool empty() const { return count == 0; }
    int treeHeight() const { return height(root); }

    // For testing only: true if every node satisfies the BST and AVL invariants.
    bool isValid() const { return validate(root, nullptr, nullptr) >= 0; }
};

#endif
