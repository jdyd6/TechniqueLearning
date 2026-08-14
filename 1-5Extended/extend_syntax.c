#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTION_1 0
#define SECTION_2 0
#define SECTION_3 0
#define SECTION_4 1
#define SECTION_5 0


/*宏定义为何要用很多括号？*/
#if SECTION_1

#define add(a, b) a + b
/*这是典型的不标准的宏定义，当有优先级高的运算接入的时候，就会出错*/
#define add2(a, b) (a + b)
/*这同样是不标准的，因为当参数a,b之中有表达式，而这个表达式中有
优先级更高的运算符时就会出错。例如：add2(3 > 1? 3 : 1, 2) 会被展开为 (3 > 1? 3 : 1 + 2），
因为先算术后比较，所以结果为 (3 > 1 ? 3 : (1 + 2)), 结果为3.而预期结果为 5
*/

# define add3(a, b) ((a) + (b))
/*这是标准的，因为将参数用括号括起来，可以保证运算的优先级*/

#define add4(a, b) ({ \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    _a + _b; \
})
/*这样定义可以兼容表达式，++ --之类的结果也对*/


#endif


/*GNU（gcc是GNU C Compiler的缩写，是windows平台下最常用的C语言编译器） 扩展语法*/
#if SECTION_2
int a = 3;
typeof(a) b = 6;

int fun (typeof(a) x)
{
    return x * x ;
}

typeof(fun(b)) c = 66;
/*typeof 是GNU C 扩展语法，用于获取变量的类型
返回值是变量的类型，而不是变量的值。
typeof(a) b = 6;  //b的类型是int
typeof(fun(b)) c = 66;  //c的类型是int.
typeof的参数为函数时，返回值为函数的返回值类型。
*/

#define max(x, y)  ({ \
    typeof(x) _x = (x); \
    typeof(y) _y = (y); \
    _x > _y ? _x : _y; \
})



#endif


#if SECTION_3

#define min_t(type, x, y) ({ \
    type _x = (x); \
    type _y = (y); \
    _x < _y ? _x : _y; \
})
/*
这里的type是如何作用于两个变量的？
type也是宏的一个参数，用于后面展开
*/
#endif

/*零长度数据 -- 结构体上的伸缩尾巴接口，就像一个房区指示牌，不占用空间，但是指示了一个区域*/
#if SECTION_4

struct Room_area{
    int len;
    char instruction[0];
};



#endif




int main(void)
{
    #if SECTION_1
    int result = add(1, 2);
    printf("Result: %d\n", result);

    int result2 = 3 * add(1, 2);
    printf("Result2: %d\n", result2);
    //结果不是预期9， 而是5.因为 3 * add(1, 2) 会被展开为 3 * 1 + 2，而不是 3 * (1 + 2)

    int result3 = 3 * add2(1, 2);
    printf("Result3: %d\n", result3);

    int result4 = add2(3 > 1? 3 : 1, 2);
    printf("Result4: %d\n", result4);

    int result5 = add3(3 > 1? 3 : 1, 2);
    printf("Result5: %d\n", result5);

    int i = 5;
    int j = 6;
    int result6 = add(i++, j++);
    int result7 = add4(i++, j++);
    printf("Result6: %d\n", result6);
    printf("Result7: %d\n", result7);
    printf("i: %d, j: %d\n", i, j);

    #endif

    #if SECTION_2
    printf("a: %d, b: %d, c: %d\n", a, b, c);
    printf("max(3, 4): %d\n", max(3, 4));
    #endif

    #if SECTION_3
    printf("min_t(float, 3.6, 4.9): %f\n", min_t(float, 3.6, 4.9));
    #endif

    #if SECTION_4
    struct Room_area* p = malloc(sizeof(*p) + 20);
    p->len = 10;
    strcpy(p->instruction, "Gaming room");
    printf("p->len: %d, p->instruction: %s\n", p->len, p->instruction);
    free(p);
    //注意使用malloc和free时，需要包含stdlib.h头文件
    //注意使用strcpy时，需要包含string.h头文件
    #endif


    return 0;
}