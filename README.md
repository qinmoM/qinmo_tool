# C++11 Tool Collection

- This is my light-weight C++11 tool library.

## Tutorial

## Install

- Clone this library into your third-party folder.

```shell
git clone -b main --single-branch https://github.com/qinmoM/qinmo_tool
```

- Append the following lines to the end of your `CMakeLists.txt`.

```cmake
add_subdirectory(
    # third-party path  +  qinmo_tool
    # example: external/qinmo_tool
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        qinmo_tool
)
```

- Include header.

```cpp
#include <qinmo/tools.h>
```

---
> The library is cross-platform, Windows and Linux.  
> Welcome to discuss.  
