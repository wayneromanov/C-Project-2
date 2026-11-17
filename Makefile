CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -g

.PHONY: all clean test

all: cachetree

cachetree: main.cpp AVLTree.h MinHeap.h
	$(CXX) $(CXXFLAGS) main.cpp -o cachetree

# Not required by the assignment: builds and runs the unit and CLI tests.
test: cachetree test_avl test_heap
	./test_avl
	./test_heap
	./test_cli.sh

test_avl: test_avl.cpp AVLTree.h
	$(CXX) $(CXXFLAGS) test_avl.cpp -o test_avl

test_heap: test_heap.cpp MinHeap.h
	$(CXX) $(CXXFLAGS) test_heap.cpp -o test_heap

clean:
	rm -rf cachetree test_avl test_heap *.o *.dSYM
