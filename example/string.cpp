#include <qinmo/tool.h>

int main()
{
    // concat
    std::cout << qinmo::concat(1, 't', "h\n");
    // println
    qinmo::println(2, 't', "h");

    // StringView
    qinmo::StringView view("hello");
    qinmo::println(view, " size:", view.size(), " index_1:", view[1]);

    return 0;
}