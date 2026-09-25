#include <qinmo/tool.h>

int main()
{
    int arr[] = { 1, 2, 3 };
    qinmo::Span<int> span(arr, 3);
    qinmo::println(span.size());

    // copy
    qinmo::Span s2 = span;
    // access
    qinmo::println(span[0]);
    qinmo::println(span.at(1));
    qinmo::println(span.back());


    // iterator
    for (int i = 0; i < span.size(); ++i)
        qinmo::print(span.at(i), ' ');

    qinmo::println();

    span[0] = 0;
    // range-for
    for (auto i : s2)
        qinmo::print(i, " ");

    /*
        output:

        3
        1
        2
        3
        1 2 3 
        0 2 3
    */

    return 0;
}