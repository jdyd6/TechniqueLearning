/*
编译：
g++ -std=c++17 -Wall -Wextra template_demo.cpp -o temp

*/



#include <cstdio>
#include <iostream>
#include <cstdint>


#define IF(O, A, B) ((O) ? (A) : (B))

template <bool, class L, class R> struct IF {typedef R type;};
//这里是通用模板，没有特化时走这里
template <class L, class R> struct IF<true, L, R> {typedef L type;};
//这里是偏特化，条件为true时，使用L类型


#define bits_t(N) typename IF<((N) > 32), uint64_t, typename IF<((N) > 16), uint32_t, typename IF<((N) > 8), uint16_t, uint8_t>::type>::type>::type

template <int N>
void demo(){
    bits_t(N) a = 10;
    std::cout << "size of a is " << sizeof(a) << std::endl;
}
/*
- 注意bits_t是一个宏，并不是一个模板，不能用<>
- bits_t(N) 定义中用了typename关键字，所以这里的N使用的场景就是不确定N是什么类型时
- IF<((N) > 32), 要加一个括号，否则大于符号 > 会被当成模板实参列表结束的标志
*/


//类模板
template <typename T>
class MyClass{
    public:
        T data;
        MyClass(T d): data(d){
            std::cout << "the data is " << data << std::endl;
        }
 
};




int main(){
    
    demo<11>();
    std::cout << IF(1, 66, 0) << std::endl;


    MyClass<int> xclass(66);
    MyClass<double> yclass(3.14);
    return 0;
}

/*

*/