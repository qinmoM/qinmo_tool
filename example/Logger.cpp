#include <qinmo/tool.h> // <qinmo/tool/Logger.h>

int main()
{
    QINMO_TRACE("A");
    QINMO_DEBUG("B");
    QINMO_INFO("C");
    QINMO_WARN("D");
    QINMO_ERROR("E");
    QINMO_FATAL("F");

    /*
    output:

    [UTC:2026-11-04 16:45:15.025846] [TID:OS:45896] [TRACE] A - D:\code_VSCode\CMake\code016_qinmo_tool\main.cpp:489
    [UTC:2026-11-04 16:45:15.027681] [TID:OS:45896] [DEBUG] B - D:\code_VSCode\CMake\code016_qinmo_tool\main.cpp:490
    [UTC:2026-11-04 16:45:15.031731] [TID:OS:45896] [INFO] C - D:\code_VSCode\CMake\code016_qinmo_tool\main.cpp:491
    [UTC:2026-11-04 16:45:15.031763] [TID:OS:45896] [WARN] D - D:\code_VSCode\CMake\code016_qinmo_tool\main.cpp:492
    [UTC:2026-11-04 16:45:15.031784] [TID:OS:45896] [ERROR] E - D:\code_VSCode\CMake\code016_qinmo_tool\main.cpp:493

    */

    return 0;
}