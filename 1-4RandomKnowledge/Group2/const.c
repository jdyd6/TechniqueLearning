#include <stdio.h>
#include "const.h"

#define SECTION_1 0
#define SECTION_2 1
#define SECTION_3 0


#if SECTION_1
static const int *p = NULL;
static int a = 10;

void TestConst(void)
{
    p = &a;
    //*p = 20;  //error, const int *p is a const pointer, cannot be modified
    printf("a = %d\n", (*p));
}

#endif

#if SECTION_2

static const int data_read = 666;

void TestArgument(const int *data)
{
    //*data+=100; //try to modify the data, but it is a const pointer, so it is not allowed
    int data_get = *data;

    //static data_get = *data; //error, static variable must be initialized with a constant expression

    
    printf("data_get = %d\n", data_get);
    printf("data_read = %d\n", data_read);
}


#endif





int main(void)
{
    #if SECTION_2
    TestArgument(&data_read);

    #endif
    
}
