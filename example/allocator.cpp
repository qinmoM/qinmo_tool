#include <qinmo/tool.h> // <qinmo/tool/allocator.h>

int main()
{
    qinmo::allocator<int> allo;
    int* ptr = allo.allocate(5);

    for (int i = 0; i < 5; ++i)
        ptr[i] = i;
    for (int i = 0; i < 5; ++i)
        qinmo::println(ptr[i]);

    allo.deallocate(ptr, 5);

    /*
    output:

    0
    1
    2
    3
    4
    */

    return 0;
}