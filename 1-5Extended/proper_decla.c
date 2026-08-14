/*
 * GNU __attribute__ 属性声明速览（GCC/Clang 扩展，非传统标准 C）
 * 编译：gcc proper_decla.c -Wall -o proper_decla.exe
 * 运行：./proper_decla.exe  （先看到 constructor，再看到 main，最后 destructor）
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>   /* offsetof */
#include <stdint.h>   /* uintptr_t */
#include <stdarg.h>

/* ========== 1. packed：去掉成员间 padding（协议包/硬件寄存器布局） ========== */
struct Normal {
    char a;
    int  b;
};

struct Packed {
    char a;
    int  b;
} __attribute__((packed));

/* ========== 2. aligned：强制变量按 N 字节对齐 ========== */
int g_aligned __attribute__((aligned(16)));

/* ========== 3. section：放入指定链接段（启动表、自定义数据段） ========== */
static const int g_in_section __attribute__((section(".mydata"))) = 42;

/* ========== 4. noreturn：函数永不返回，优化/告警更准 ========== */
static void die(const char *msg) __attribute__((noreturn, unused));
static void die(const char *msg)
{
    fprintf(stderr, "fatal: %s\n", msg);
    exit(1);
}

/* ========== 5. constructor / destructor：main 前后自动执行 ========== */
static void boot(void) __attribute__((constructor));
static void boot(void)
{
    printf("[constructor] main 还没开始\n");
}

static void shutdown(void) __attribute__((destructor));
static void shutdown(void)
{
    printf("[destructor] main 已经结束\n");
}

/* ========== 6. unused：允许定义后不用，抑制警告 ========== */
static void touch_unused(void)
{
    int only_for_debug __attribute__((unused)) = 0;
}

/* ========== 7. format：按 printf 规则检查格式串与后面参数 ========== */
static void my_log(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void my_log(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

/* ========== 8. warn_unused_result：返回值不该被丢掉 ========== */
static int must_check(void) __attribute__((warn_unused_result));
static int must_check(void)
{
    return 7;
}

/* ========== 9. nonnull：指定参数不应为 NULL ========== */
static int len_str(const char *s) __attribute__((nonnull(1)));
static int len_str(const char *s)
{
    int n = 0;
    while (s[n])
        n++;
    return n;
}

/* ========== 10. weak：弱符号，可被别处的强定义覆盖 ========== */
void __attribute__((weak)) plugin_hook(void)
{
    printf("[weak] 使用默认实现\n");
}

/* ========== 11. always_inline / noinline：强制内联 / 禁止内联 ========== */
static inline int add_fast(int a, int b) __attribute__((always_inline));
static inline int add_fast(int a, int b)
{
    return a + b;
}

static int add_slow(int a, int b) __attribute__((noinline));
static int add_slow(int a, int b)
{
    return a + b;
}

/* ========== 12. deprecated：调用时提醒迁到新接口 ========== */
static void old_api(void) __attribute__((deprecated("请改用 new_api"), unused));
static void old_api(void)
{
    printf("[deprecated] old_api\n");
}

static void new_api(void)
{
    printf("[ok] new_api\n");
}

int main(void)
{
    printf("sizeof(Normal)=%zu, sizeof(Packed)=%zu\n",
           sizeof(struct Normal), sizeof(struct Packed));
    printf("offsetof(Packed, b)=%zu  （packed 下通常为 1）\n",
           offsetof(struct Packed, b));

    /* aligned(16)：地址应对 16 对齐（取模为 0） */
    printf("g_aligned 地址=%p, 对16取模=%zu\n",
           (void *)&g_aligned, (size_t)((uintptr_t)&g_aligned % 16));

    printf("section 变量 g_in_section=%d\n", g_in_section);

    touch_unused();
    my_log("format 示例: %d\n", add_fast(1, 2));
    /* 故意写错类型时，-Wall 常能靠 format 属性抓出来： */
    /* my_log("bad: %s\n", 123); */

    int v = must_check(); /* 若写成 must_check(); 可能触发 warn_unused_result */
    printf("must_check=%d, add_slow(3,4)=%d, len=%d\n",
           v, add_slow(3, 4), len_str("hi"));

    plugin_hook();
    new_api();
    /* 取消注释 + -Wall：可见 deprecated 警告 */
    /* old_api(); */

    /* 取消注释会直接退出进程（演示 noreturn） */
    /* die("demo"); */

    return 0;
}
