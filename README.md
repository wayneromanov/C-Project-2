CSCE 2110: Computing Foundations II (Data Structures)

Programming Assignment 4: Self-Balancing Indices & Priority Queries (CacheTree Engine)

Assigned: Tuesday, November 3, 2026

Due Date: Friday, November 20, 2026, at 11:59 PM CST (Before Thanksgiving Break)

Course Objective & Motivation

Throughout the first half of the semester, we explored linear data structures (singly/doubly linked lists, array-based stacks, and FIFO queues). While simple to implement, these structures exhibit poor operational complexity ($O(n)$) when searching or retrieving ordered ranges within large datasets.

In this assignment, you will transition to non-linear, hierarchical memory structures. You are tasked with developing CacheTree, a memory-efficient, terminal-driven key-value query engine backed by a Self-Balancing Binary Search Tree (AVL Tree) and an auxiliary Array-Backed Min-Heap. This project evaluates your mastery of dynamic memory allocation, pointer manipulation, recursive tree traversals, tree rotations, and algorithmic complexity.

Constraints & Academic Integrity Policy

Forbidden Standard Containers: You are strictly prohibited from utilizing <map>, <unordered_map>, <set>, <unordered_set>, <queue>, <deque>, or std::priority_queue.
Permitted Standard Libraries: You may use <iostream>, <string>, <vector> (solely to store results returned by range queries and to back your internal heap array), <fstream>, <sstream>, <cassert>, and <algorithm> (for std::max or std::swap).
Language Standards: Your code must compile cleanly on university Linux servers using g++ -std=c++17 -Wall -Wextra -Werror. Warnings will be treated as compile-time errors.
Memory Leak Tolerance: Zero. Your program will be executed through Valgrind. Memory leaks, use-after-free, or double-free errors will incur severe point deductions.
System Architecture & Technical Specifications

You must implement two generic (templated) container classes alongside a driver interface.

AVL Tree Implementation (
AVLTree<K, V>)

Your tree must store key-value pairs (K key, V value), keeping keys ordered according to operator<.

Node Structure: Each node must maintain pointers to its left and right children, its stored key and value, and an integer height.
Balance Factor: For any node $N$, $BF(N) = \text{height}(N\text{->left}) - \text{height}(N\text{->right})$. The invariant $\vert{}BF(N)\vert{} \le 1$ must hold across all nodes following any mutation.
Rotations: You must implement all four rotation mechanics:
Single Right Rotation (Left-Left imbalance)
Single Left Rotation (Right-Right imbalance)
Left-Right Double Rotation (Left-Right imbalance)
Right-Left Double Rotation (Right-Left imbalance)
Required Operations:
void insert(const K& key, const V& value): Inserts a key-value pair. If the key already exists, overwrite the value. Balance must be restored on recursion unwind. Run time: $O(\log n)$.
bool remove(const K& key): Removes the specified key. If deleting a node with two children, replace it with its in-order successor (smallest key in the right subtree). Balance must be restored up the call stack. Run time: $O(\log n)$.
bool search(const K& key, V& outValue): Retrieves the associated value. Run time: $O(\log n)$.
std::vector<std::pair<K, V>> rangeQuery(const K& low, const K& high): Returns all entries where $low \le key \le high$ in ascending order. Crucial: You must prune unreachable subtrees during traversal to guarantee $O(k + \log n)$ time, where $k$ is the number of keys reported.
Rule of Three / Resource Management: Destructor (~AVLTree()), Copy Constructor, and Copy Assignment Operator (operator=) must perform deep copies and cleanly deallocate every heap-allocated node.
2. Min-Heap Priority Queue (MinHeap<T>)

An internal, array-backed binary min-heap used to efficiently compute the "Top-$K$" most frequent items without sorting the entire dataset.

Must be backed by a contiguous dynamic array or std::vector<T>.
Must implement zero-based index arithmetic for parent/child navigation:
$\text{parent}(i) = \lfloor (i - 1) / 2 \rfloor$
$\text{left}(i) = 2i + 1$
$\text{right}(i) = 2i + 2$
Operations:
void push(const T& element): Inserts an element via sift-up in $O(\log n)$.
T pop(): Removes and returns the root element via sift-down in $O(\log n)$.
const T& peek() const: Returns the root element in $O(1)$.
size_t size() const and bool empty() const.
3. Command Line Interface (CLI Driver)

Your executable (cachetree) must accept commands via standard input (std::cin) in an interactive REPL loop:

CommandArgumentsExpected Output / BehaviorPUT<string: key> <int: val>Inserts or updates the record. Prints OK.GET<string: key>Prints the integer value, or KEY_NOT_FOUND if missing.DEL<string: key>Deletes the record. Prints DELETED or KEY_NOT_FOUND.RANGE<string: low> <string: high>Prints matching key: value lines in ascending order.TOP<int: k>Uses MinHeap to find and print the top $k$ keys with the highest integer values. Runs in $O(n \log k)$.LOAD<string: filename>Populates the tree from a space-delimited text file containing key value lines.EXIT(none)Frees all dynamic memory and terminates the program with return code 0.

Rubric & Grading Breakdown (100 Points Total)

CategoryDescriptionPointsAVL Core & RotationsAccurate balance factor calculation and correct execution of all 4 rotation scenarios (LL, RR, LR, RL) during sequential and random inserts.25 ptsNode Deletion LogicProper handling of 0-child, 1-child, and 2-child node removal with successor swapping and subsequent rebalancing up to the root.20 ptsRange Search PruningRange query properly prunes invalid subtrees; verified to not degrade to an unconditional $O(n)$ full in-order traversal.15 ptsMin-Heap Top-$K$ EngineCorrect sift-up/sift-down heap mechanics; correctly streams items through a size-$K$ heap to output the top elements.15 ptsMemory Cleanliness (Valgrind)Clean execution under Valgrind test suites. 0 leaks, 0 dangling references, correct recursive destructor and copy semantics.15 ptsCLI & File ParsingRobust parsing of CLI commands, edge-case validation (e.g., empty tree, duplicate inserts, missing files), and output formatting.10 ptsLate PenaltySubmitted after deadline: -10 points per 24 hours. No submissions accepted after Monday, November 23 at 11:59 PM.—

Submission Guidelines

Submit a single .zip file to Canvas named euid_pa4.zip containing:

AVLTree.h (Templated class definition and implementation)
MinHeap.h (Templated class definition and implementation)
main.cpp (REPL driver and file handling)
Makefile (Must include targets: all, cachetree, and clean)
README.txt (Brief instructions on how to build, any known bugs, and timing complexity notes) 
