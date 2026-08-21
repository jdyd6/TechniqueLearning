#include <cstdio>
// #include "mangling_test.h"

extern "C"{
    int max(int a, int b);
}
/*声明一个C函数，这样做可以不需要头文件。
使用于第三方的C头文件没法修改时*/



int main() {
    int a = 10;
    int b = 20;
    int result = max(a, b);
    std::printf("max(%d, %d) = %d\n", a, b, result);
    return 0;
}
