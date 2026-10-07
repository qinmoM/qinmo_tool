#include <qinmo/tool.h> // <qinmo/tool/make_unique.h>

int main()
{
    // construct object
    auto strPtr = qinmo::make_unique<std::string>("hello");
    qinmo::println(*strPtr);

    // construct dynamic array
    auto arr = qinmo::make_unique<int[]>(10);
    arr[1] = 180;
    qinmo::println(arr[1]);

    // do not use fixed-size array
    // qinmo::make_unique<int[10]>();


    /*
        output:

        hello
        180
    */

    return 0;
}