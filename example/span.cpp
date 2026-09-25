#include <qinmo/tool.h>

int main()
{
    int arr[] = { 1, 2, 3 };
    qinmo::Span<int> span(arr, 3);
    qinmo::println("the size of span: ", span.size());

    // copy
    qinmo::Span s2 = span;
    // access
    qinmo::println("the first digit of array: ", span[0]);
    qinmo::println("the second digit of array: ", span.at(1));
    qinmo::println("the third digit of array: ", span.back());


    // iterator
    for (int i = 0; i < span.size(); ++i)
        qinmo::print(span.at(i), ' ');

    qinmo::println();

    span[0] = 0;
    // range-for
    for (auto i : s2)
        qinmo::print(i, " ");

    qinmo::println();
    // view
    qinmo::println(span.subspan(1, 1).at(0));

    /*
        output:


        the size of span: 3
        the first digit of array: 1
        the second digit of array: 2
        the third digit of array: 3
        1 2 3 
        0 2 3 
        2

    */

    return 0;
}