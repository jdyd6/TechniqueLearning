#include "declaration.h"

// #define SECTION_1 1
// #define SECTION_2 0
// #define SECTION_3 0
// #define SECTION_4 0
/*每个.c文件的预处理是独立的，但是这样同名容易造成困扰，所以要么共享一套宏开关
要么就是用不同的名字*/

#define test_section_1 1
#define test_section_2 1
#define test_section_3 0
#define test_section_4 0

#if test_section_1

int x = 100, y = 600;

#endif

#if test_section_2

// inline int prime_number(int n) {
//     if (n <= 1) {
//         return 0;
//     }
//     for (int i = 2; i * i <= n; i++) {
//         if (n % i == 0) {
//             return 0;
//         }
//     }
//     return 1;
// }


#endif

