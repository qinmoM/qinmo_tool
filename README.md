# C++11 Tool Collection

- This is my light-weight C++11 tool library.

## Tutorial

```cpp
#include <qinmo/tool.h>

int main()
{
    // print
    qinmo::println(qinmo::RandomInt(0, 3), 't', "h"); // output: 2th

    // string view
    qinmo::StringView view("hello");
    std::cout << view << std::endl;
    std::cout << view[1] << std::endl;

    return 0;
}
```

[See details.](./example/)

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
