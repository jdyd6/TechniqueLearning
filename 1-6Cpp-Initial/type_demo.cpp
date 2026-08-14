/*
从C转到C++，要切换的是资源管理的思路，C++的思想是 RALL -- Resource Acquisition Is Initialization,
资源获取即初始化，把资源（内存、文件句柄、数据库连接等）的获取和释放与对象的生命周期绑定在一起，
避免资源泄露

*/


#include <iostream>


#define SECTION_1 1
#define SECTION_2 0
#define SECTION_3 0


/*g++编译*/
#if SECTION_1
int num = 66;

/*
g++编译的标准格式：
g++ -std=c++17 -Wall -Wextra -o xxx -xxx.cpp
其中-std=c++17 表示使用c++17标准，-Wall 表示启用所有警告，-Wextra 表示启用额外警告
xxx在Windows环境下就是xxx.exe可执行文件，Wall-- waring all，Wextra-- waring extra
*/
#endif

#if SECTION_2

#endif



int main() {

    #if SECTION_1
    std::cout << "num = " << num << std::endl;
    #endif

    return 0;
}