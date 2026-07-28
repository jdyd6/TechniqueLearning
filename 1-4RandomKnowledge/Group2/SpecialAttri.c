#include <stdio.h>
#include "SpecialAttri.h"


#define SECTION_1 0
#define SECTION_2 1
#define SECTION_3 0
#define SECTION_4 0


/*weak funciton*/
#if SECTION_1

void __attribute__((weak)) WeakFun(void) {
    printf("This is a weak function.\n");
}

#endif // SECTION_1


#if SECTION_2

int __attribute__((weak)) data = 100;


#endif // SECTION_2



int main(void){

    #if SECTION_1
    WeakFun();

    #endif // SECTION_1


    #if SECTION_2
    int data = 666;
    printf("data = %d\n", data); //the output is 666, because we define a strong variable in main function, so the weak variable is shadowed by the strong variable.
    #endif // SECTION_2


    return 0;
}
