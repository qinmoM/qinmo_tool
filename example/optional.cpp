#include <qinmo/tool.h> // <qinmo/tool/Optional.h>

int main()
{
    using namespace qinmo;

    // assignment
    Optional<int> op = nullopt;
    op.emplace(10);
    op = 10.0;
    if (op)
        println(*op);

    // copy
    Optional<double> opt = op;
    if (op.has_value())
        println(op.value());

    // modifiers
    opt.reset();
    println(opt.value_or(999));

    // monadic
    Optional<std::string> strOp;
    Optional<std::string> res = strOp
        .or_else(
            []()
            {
                return Optional<std::string>("abcd.");
            })
        .transform(
            [](const std::string& str)
            {
                return "string is " + str;
            }
        )
        .and_then(
            [](const std::string& str)
            {
                println(str);
                return Optional<std::string>(str);
            }
        );

    /*
        output:

        10
        10
        999
        string is abcd.

    */

    return 0;
}