#include <iostream>
#include <algorithm>

#define MAX_NUM (10)

int main() {
    int array[MAX_NUM] = {0};

    array[3] = 8;
    int *ip = std::find(array, array + MAX_NUM, 8);

    if (ip == array + MAX_NUM) {
        std::cout << "Not found " << 8 << " in array!" << std::endl;
    }
    else {
        std::cout << "Found it." << std::endl;
    }
    return 0;
}
