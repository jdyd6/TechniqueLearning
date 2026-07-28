#include <stdio.h>
#include "declaration.h"


// #define NDEBUG   //if define NDEBUG, the assert will be ignored.



/*sections practice control*/
#define SECTION_1 0
#define SECTION_2 0
#define SECTION_3 0
#define SECTION_4 0
#define SECTION_5 0
#define SECTION_6 0
#define SECTION_7 0
#define SECTION_8 0
#define SECTION_9 0
#define SECTION_10 0


// int array[10] = {1, 3, 5, 7, 9, 11, [0] = 2, 4, 6, 8};
/*输出结果为2 4 6 8 1 3 5 7 9 11， 表明初始化的时候可以指定任何元素，后面依次初始化
下标递增的值，比如[0] = 2, 表示第一个元素为2，后面依次初始化. 但是似乎没有什么价值，
因为为什么要这样别扭的初始化呢？*/

#define ARM 1

//选择环境
#if SECTION_1
void platform_init(void)
{
    #ifdef ARM
    printf("Running on ARM\n");
    #elif defined LINUX
    printf("Running on LINUX\n");
    #elif defined WINDOWS
    printf("Running on WINDOWS\n");
    #else
    printf("Running on Unknown\n");
    #endif


    // #ifdef LINUX || ARM || WINDOWS
    /*这样写只会警告不会报错，但是具体取值是def后面的第一个值， 所以后续的无效的，并没有达到“或”的逻辑
    因此用下面的defined才对*/
    #if defined(ARM) || defined(LINUX) || defined(WINDOWS)
    /*defined instrution could involve multiple argument, when any of the argument is defined, the condition is true.
    #ifdef can only involve one argument */
    printf("Ruing on a acceptable platform\n");
    #else
    printf("Running on an unacceptable platform\n");
    #endif
}
#endif

/*错误处理*/
#if SECTION_2
#include <assert.h>
// typedef enum {
//     sucess = 0,
//     warning = 1,
//     error = 2,
// } error_t;
void error_handle(int test) {
    assert((test > 0) && (test < 100));
    printf("test is %d\n", test);
    /*if the assertion failed, the program will be terminated in this line, and the printf has no chance to be executed*/
}
#endif


/*优化函数调用的额外开销 —— 保存现场， 复制参数， 跳转， 返回。
-C89中只有一种方法，就是函数宏；
-C99提供了**内联函数**。
两种方法各有利弊，宏定义不检查参数，会带来未知的问题； 内联函数 不会有这种问题，但是是
通过复制函数到需要执行的地方，每个调用的地方都复制一份，代码体积会随之增大*/
#if SECTION_3
#define SWAP(a,b) {int temp = a; a = b; b = temp;}
static inline int max(int a, int b) {
    return (a > b) ? a : b;
}
#endif

/*内联函数，外部链接调用*/
#if SECTION_4
void test_prime_number(int n) {
    if (prime_number(n)) {
        printf("%d is a prime number\n", n);
    } else {
        printf("%d is not a prime number\n", n);
    }
}

#endif


/*声明相关练习题：
1. char (*x[10])(int);
2.int (*x(int))[5];
3.float *(*x(void))(int);
4.void (*x(int, void(*y)(int)))(int);

以3为例， 说明一下如何解读：
- 先找变量名
- 抓住优先级： () [] 优先级高，并且从左到右结合
故：
    x 是一个函数， 这个函数不接收参数， 返回一个指针， 指向一个另一个函数B， 函数B接收int型参数，返回一个float型指针


*/


// #define ARM 1
/*定义在这里或者main函数里面都是unknown, 因为编译首先进行预处理，此时ARM还没有定义，所以是unknown；
就算定义在后面也不行，因为编译器是逐行编译的，所以预处理的时候ARM还没有定义，所以是unknown；*/

int main() {

    #if SECTION_1
    // #define ARM 1
     platform_init();
    #endif

    #if SECTION_2
    error_handle(200); //if define NDEBUG, the assert will be ignored. And the check will not be executed.
    #endif

    #if SECTION_3
    static int a = 10, b = 60;
    SWAP(a, b);
    printf("a is %d, b is %d\n", a, b);

    // for (int i = 0; i < 10; i++) {
    //     for (int j = 0; j < 10; j++) {
    //         int max_value = max(i, j);
    //         printf("max value is %d\n", max_value);
    //         /*这里在同一个地方调用，看不出inline函数会导致代码体积变大的特点，反而优化了执行导致代码体积变小了，
    //         */
    //     }
    // }     
    
    SWAP(x, y);
    printf("x is %d, y is %d\n", x, y);
    /*x和y可以直接调用，因为已经声明在.h文件中， 定义在.c文件中， mian.c中不需要再extern, 
    重复extern也不会有问题*/

    #endif

    #if SECTION_4
    test_prime_number(10);
    /*这里相当于把内联函数当一个普通的外部可调用的函数来用，也是可以的， 如果是declaration.h里面
    定义的，在编译器的视角会生成一个只在mian.o可见的prime_number*/

    #endif




    return 0;
}