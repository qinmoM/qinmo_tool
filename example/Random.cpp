#include <qinmo/tool.h>
#include <iostream>

int main()
{
    // Single use
    std::cout << qinmo::RandomDouble::rand(9, 31) << '\n';
    std::cout << qinmo::RandomInt16::rand(9, 31) << '\n';

    // Multiple use
    qinmo::RandomFloat rf(1, 2);
    std::cout << rf.rand() << ' ' << rf.rand() << '\n';
    qinmo::RandomUInt32 ri(1, 20);
    std::cout << ri.rand() << ' ' << ri.rand() << '\n';

    return 0;
}