#include <stdio.h>


#define TEST_1 0
#define TEST_2 1
#define TEST_3 0



#if TEST_1
typedef int int32_t;
#endif

#if TEST_2
int form_arg(int a, int b)
{
    a++;
    b++;
    return a + b;
}

#endif


#if TEST_3

#endif

int main(void)
{

#if TEST_1
    int32_t a = 10;
    printf("a = %d\n", a);

    int b = 66;
    printf("b = %d\n", b);
#endif


#if TEST_2
    printf("form_arg(1, 2) = %d\n", form_arg(1, 2));

#endif



    return 0;

}

/*
TEST_1:
用了typedef 自定义一个类型之后，原来的类型也不会失效。它们是一回事
TEST_2:
形式参数可以在函数中使用，能返回这一步计算的结果，但是形参a,b会在函数运行结束(作用域结束)时被销毁



*/