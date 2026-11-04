#include <qinmo/tool.h> // <qinmo/tool/Logger.h>

int main()
{
    QINMO_TRACE("A");
    QINMO_DEBUG("B");
    QINMO_INFO("C");
    QINMO_WARN("D");
    QINMO_ERROR("E");
    QINMO_FATAL("F");

    return 0;
}