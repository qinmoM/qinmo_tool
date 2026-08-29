#include <qinmo/tool.h> 
// equal to :
// #include <qinmo/tool/StringView.h> 
// #include <qinmo/tool/StringConcat.h>

int main()
{
    // concat
    std::cout << qinmo::concat(1, 't', "h\n");
    // println
    qinmo::println(2, 't', "h");

    // StringView
    qinmo::StringView view("hello");
    qinmo::println(view, " size:", view.size(), " index_1:", view[1]);
    std::string str = view.to_string();

    return 0;
}