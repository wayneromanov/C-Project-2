// CacheTree: a terminal key-value query engine backed by an AVL tree,
// with a min-heap for top-K queries. Reads commands from standard input.

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "AVLTree.h"
#include "MinHeap.h"

namespace {

// One candidate for TOP k. "Smaller" means "ranks lower", so the heap's root is always
// the weakest of the current top k: the lowest value, and among equal values the key
// that comes last alphabetically. Ties are therefore reported in alphabetical order.
struct Entry {
    std::string key;
    int value;

    bool operator<(const Entry& other) const {
        if (value != other.value) return value < other.value;
        return other.key < key;
    }
};

// Parses a whole token as an int. Rejects "12abc", "abc", and numbers that don't fit in an
// int (the stream sets its fail state on overflow).
bool parseInt(const std::string& s, int& out) {
    std::istringstream ss(s);
    int v;
    char extra;
    if (!(ss >> v) || (ss >> extra)) return false;
    out = v;
    return true;
}

// Commands are matched case-insensitively ("put" works too); keys stay case-sensitive.
std::string toUpper(std::string s) {
    for (char& c : s)
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
    return s;
}

// Prints the k keys with the highest values, highest first, in O(n log k):
// every entry streams past a heap that never holds more than k items.
void topK(const AVLTree<std::string, int>& tree, int k) {
    if (k <= 0 || tree.empty()) return;
    std::size_t limit = static_cast<std::size_t>(k);

    MinHeap<Entry> heap;
    tree.forEach([&](const std::string& key, int value) {
        Entry e{key, value};
        if (heap.size() < limit)
            heap.push(e);
        else if (heap.peek() < e) {     // beats the weakest of the current top k
            heap.pop();
            heap.push(e);
        }
    });

    // Popping yields lowest first, so fill the array from the back to print highest first.
    std::size_t m = heap.size();
    Entry* ranked = new Entry[m];
    for (std::size_t i = m; i > 0; --i) ranked[i - 1] = heap.pop();
    for (std::size_t i = 0; i < m; ++i) std::cout << ranked[i].key << ": " << ranked[i].value << '\n';
    delete[] ranked;
}

// Reads "key value" lines into the tree. Blank and malformed lines are skipped.
// Returns false if the file can't be opened.
bool loadFile(AVLTree<std::string, int>& tree, const std::string& filename, int& loaded, int& skipped) {
    std::ifstream in(filename);
    if (!in) return false;
    loaded = skipped = 0;
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        std::string key, valueText, extra;
        if (!(ss >> key)) continue;     // blank line
        int value;
        if (!(ss >> valueText) || (ss >> extra) || !parseInt(valueText, value)) {
            ++skipped;
            continue;
        }
        tree.insert(key, value);
        ++loaded;
    }
    return true;
}

}  // namespace

int main() {
    AVLTree<std::string, int> tree;
    std::string line;

    while (std::getline(std::cin, line)) {
        std::istringstream ss(line);
        std::string cmd;
        if (!(ss >> cmd)) continue;     // ignore blank lines

        std::vector<std::string> args;
        std::string tok;
        while (ss >> tok) args.push_back(tok);
        cmd = toUpper(cmd);

        if (cmd == "PUT") {
            int value;
            if (args.size() != 2 || !parseInt(args[1], value)) {
                std::cout << "ERROR: usage PUT <key> <int>\n";
                continue;
            }
            tree.insert(args[0], value);
            std::cout << "OK\n";
        } else if (cmd == "GET") {
            if (args.size() != 1) {
                std::cout << "ERROR: usage GET <key>\n";
                continue;
            }
            int value;
            if (tree.search(args[0], value))
                std::cout << value << '\n';
            else
                std::cout << "KEY_NOT_FOUND\n";
        } else if (cmd == "DEL") {
            if (args.size() != 1) {
                std::cout << "ERROR: usage DEL <key>\n";
                continue;
            }
            std::cout << (tree.remove(args[0]) ? "DELETED\n" : "KEY_NOT_FOUND\n");
        } else if (cmd == "RANGE") {
            if (args.size() != 2) {
                std::cout << "ERROR: usage RANGE <low> <high>\n";
                continue;
            }
            for (const auto& kv : tree.rangeQuery(args[0], args[1]))
                std::cout << kv.first << ": " << kv.second << '\n';
        } else if (cmd == "TOP") {
            int k;
            if (args.size() != 1 || !parseInt(args[0], k) || k < 0) {
                std::cout << "ERROR: usage TOP <k>, k >= 0\n";
                continue;
            }
            topK(tree, k);
        } else if (cmd == "LOAD") {
            if (args.size() != 1) {
                std::cout << "ERROR: usage LOAD <filename>\n";
                continue;
            }
            int loaded = 0, skipped = 0;
            if (!loadFile(tree, args[0], loaded, skipped)) {
                std::cout << "ERROR: cannot open file " << args[0] << '\n';
                continue;
            }
            std::cout << "LOADED " << loaded;
            if (skipped > 0) std::cout << " (skipped " << skipped << " malformed lines)";
            std::cout << '\n';
        } else if (cmd == "EXIT") {
            break;
        } else {
            std::cout << "ERROR: unknown command " << cmd << '\n';
        }
    }
    // The tree's destructor frees every node when main returns (EXIT or end of input).
    return 0;
}
