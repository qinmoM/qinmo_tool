#include <qinmo/tool.h> 
#include <algorithm>
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
    qinmo::StringView sv = "hello, hello.";
    std::string str = sv.to_string();
    std::cout << sv << std::endl;

    // access
    qinmo::println(sv.at(8));
    qinmo::println(sv[8]);
    qinmo::println(sv.front());

    // search
    qinmo::println(sv.find("."));

    if (qinmo::StringView::npos == sv.rfind("9."))
        qinmo::println("non-found");
    else
        qinmo::println("found");

    qinmo::println(sv.find_first_of("hpd"));

    // generic algorithm
    qinmo::println("generic algorithm count(l)=", std::count(sv.begin(), sv.end(), 'l'));
    qinmo::println("generic algorithm reverse-find(l)=", *(std::find(sv.rbegin(), sv.rend(), 'l')));

    // operations
    qinmo::println("substr 1~3: ", sv.substr(1, 3));

    if (sv.starts_with("he"))
        qinmo::println("found: he");
    else
        qinmo::println("non-found: he");

    if (sv.ends_with("lo."))
        qinmo::println("ends_with: lo.");
    else
        qinmo::println("non-ends_with: lo.");

    // modifiers
    sv.remove_suffix(8);
    qinmo::println("modifier: ", sv);

    // throw
    try
    {
        qinmo::println(sv.at(100));
    }
    catch (const std::exception& e)
    {
        qinmo::println(e.what());
    }

    /*
        output:

        1th
        2th
        hello, hello.
        e
        e
        h
        12
        non-found
        0
        generic algorithm count(l)=4
        generic algorithm reverse-find(l)=l
        substr 1~3: ell
        found: he
        ends_with: lo.
        modifier: hello
        StringView::at: index(100) out of range(0 ~ 4)
    */

    return 0;
}