#include "ptr.h"

void test() {
    parent_ptr father(new parent());
    children_ptr son(new children());

    father->children = son;
    son->parent = father;
}

void main() {
    std::cout << "begin test...\n";
    test();
    std::cout << "end test" << std::endl;
}
