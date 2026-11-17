CSCE 2110 - Programming Assignment 4: CacheTree Engine
Name: TODO
EUID: TODO

BUILD
-----
    make            builds ./cachetree (same as "make cachetree")
    make clean      removes build outputs

Compiled with: g++ -std=c++17 -Wall -Wextra -Werror -g

RUN
---
    ./cachetree                 interactive: type one command per line
    ./cachetree < commands.txt  read commands from a file

Commands (names are case-insensitive; keys are case-sensitive):
    PUT <key> <int>       insert or overwrite            -> OK
    GET <key>             look up                        -> the value, or KEY_NOT_FOUND
    DEL <key>             delete                         -> DELETED, or KEY_NOT_FOUND
    RANGE <low> <high>    keys with low <= key <= high   -> one "key: value" line each, ascending
    TOP <k>               k highest values               -> one "key: value" line each, highest first
    LOAD <filename>       read "key value" lines         -> LOADED <n>
    EXIT                  free memory and exit with code 0 (end of input does the same)

Behaviour the assignment does not specify (my choices):
  - Keys are compared as strings with operator<, so "10" < "9" and "B" < "a".
  - RANGE with no matches (or low > high) prints nothing.
  - TOP: ties on value are listed in alphabetical key order. If k is larger than the
    number of keys, all keys are listed. TOP 0 and TOP on an empty tree print nothing.
  - LOAD: blank lines are ignored; malformed lines (missing/non-integer value, extra
    fields) are skipped and reported as "LOADED <n> (skipped <m> malformed lines)".
    Existing keys are overwritten. A missing file prints
    "ERROR: cannot open file <filename>" and leaves the tree unchanged.
  - Invalid input (wrong number of arguments, a value that is not an int or does not
    fit in an int, negative k, unknown command) prints a line starting with "ERROR:"
    and the program keeps running.

FILES
-----
    AVLTree.h    templated AVL tree (insert, remove, search, rangeQuery, Rule of Three)
    MinHeap.h    templated array-backed binary min-heap (push, pop, peek, size, empty)
    main.cpp     command loop, file loading, top-K

KNOWN BUGS
----------
None known.

TIME COMPLEXITY  (n = number of keys, k = keys reported / requested)
---------------
AVLTree
    insert       O(log n)      one root-to-leaf path down, rebalance on the way up
                               (at most one rotation is needed per insert)
    remove       O(log n)      one path down (plus the walk to the in-order successor),
                               rebalance every ancestor on the way up
    search       O(log n)      iterative walk down one path
    rangeQuery   O(k + log n)  the left subtree is only entered when low < node key and
                               the right subtree only when node key < high, so only the
                               k matching nodes and the two boundary paths are visited
    copy / =     O(n)          every node is cloned (deep copy)
    destructor   O(n)          post-order delete of every node
    The height of an AVL tree is at most about 1.44 log2(n), so "log n" is guaranteed,
    unlike an unbalanced BST, which degrades to O(n) on sorted input.

MinHeap
    push         O(log n)      append, then sift up
    pop          O(log n)      move last element to the root, then sift down
    peek, size, empty   O(1)

Commands
    PUT, GET, DEL       O(log n)
    RANGE               O(k + log n)
    TOP k               O(n log k): an in-order walk streams all n entries past a heap
                        holding at most k entries; an entry only replaces the heap's root
                        (the weakest of the current top k) when it ranks higher. The k
                        results are then popped in O(k log k).
    LOAD (m lines)      O(m log(n + m))

Space: O(n) for the tree; TOP uses O(k) extra; RANGE returns O(k) results.
Recursion depth is the tree height, O(log n).

MEMORY
------
Every node is allocated with new in insert and freed with delete in remove or in the
destructor; the copy constructor and operator= deep-copy, and operator= frees the old
nodes and handles self-assignment. Checked with Valgrind:
    valgrind --leak-check=full --error-exitcode=1 ./cachetree < commands.txt
