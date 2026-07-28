#ifndef __DECLARATION_H__  //防止重复包含，否则可能会导致变量被重复定义之类的问题
#define __DECLARATION_H__


static inline int prime_number(int n) {
    if (n <= 1) {
        return 0;
    }
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}
/*头文件中的static 定义有点特殊，外部可以链接
看起来像“大家都能用”，但链接器眼里是：
practice1.o  里有一份 prime_number（static，仅本 .o 可见）
other.o      里又有一份 prime_number（static，仅本 .o 可见）
declaration.o 若也 include 了，还有第三份……*/


extern int x, y;


#endif