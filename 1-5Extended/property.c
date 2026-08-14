#include <stdio.h>


#define SECTION_1 0
#define SECTION_2 0
#define SECTION_3 0
#define SECTION_4 1
#define SECTION_5 0



#if SECTION_1
struct Book{
    char name[21];
    int price;
    int page;
    char author[21];
}__attribute__((packed));


struct Book book = {
    .name = "The Great Gatsby",
    .price = 10,
    .page = 100,
    .author = "F. Scott Fitzgerald"
};
#endif


#if SECTION_2
struct Book{
    char name[21];
    int price;
    int page;
    char author[21];
}__attribute__((aligned(16)));

struct Book book = {
    .name = "The Great Gatsby",
    .price = 10,
    .page = 100,
    .author = "F. Scott Fitzgerald"
};

/* 
aligned:强制变量按 N 字节对齐
注意不是结构体重的每个成员都要按N字节对齐，而是整个结构体按N字节对齐
*/

#endif


#if SECTION_3
static void __attribute__((constructor)) boot(void){
    printf("the main function will be called after this function\n");
}

static void __attribute__((destructor)) shutdown(void){
    printf("the main function is finished\n");
}


#endif




int main(void){
    #if SECTION_1
    printf("the size of book is %d", sizeof(book));
    //if didn't use attribute((packed)), the size is 56, because it will be aligned to 4 bytes
    //if use attribute((packed)), the size is 50, it will discard the padding of the struct
    #endif

    #if SECTION_2
    /*test the font of annotation*/
    printf("the size of book is %d", sizeof(book));
    //if use attribute((aligned(16))), the size is 64, because it will be aligned to 16 bytes
    printf("the addr of .name is %p\n", &book.name);
    printf("the addr of .price is %p\n", &book.price);
    printf("the addr of .page is %p\n", &book.page);
    printf("the addr of .author is %p\n", &book.author);
    #endif

    #if SECTION_3
    printf("the main function is running\n");

    #endif

    

    return 0;
}