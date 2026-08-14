# 第 1 周：better C，先把编译和类型拧紧

> 本周停在：能解释「为什么 C++ 不许 `int *p = malloc(...)` 不写强制转换」。
>
> 对照仓库：`1-5Extended` 的宏、`1-4` 的头文件纪律。编译器用你已经熟悉的 GCC，只是前端换成 `g++`。

```bash
g++ -std=c++17 -Wall -Wextra -o demo demo.cpp
```

---

## 知识点 1：C++ 把类型检查拧紧了一圈

### 先想这个问题

C 对你很纵容：函数可以不声明就调用，`void *` 能悄悄变成任何指针。这像超市不关门禁，推车里塞炸药也没人拦。

你在 `1-5Extended` 里用 `malloc` 给柔性数组成员开尾巴，C 允许：

```c
int *p = malloc(n * sizeof(int));  /* C：void* 自动变成 int* */
```

同一行丢进 `.cpp`，`g++` 会直接拒绝。不是 C++ 矫情，是它见过太多「把 `char` 的地址当成 `int*` 去写、把邻居内存踩烂」的事故。

### 它长什么样

```cpp
#include <cstdlib>

void demo()
{
    /* C++ 要求类型对上：void* 必须显式转成目标指针类型 */
    int *p = static_cast<int *>(std::malloc(sizeof(int)));
    std::free(p);
}
```

另外两道常见缝：

```cpp
/* C 里可以调用未声明函数；C++ 非法，必须先看到原型 */
double x = sqrt(2.0);   /* 没 #include <cmath> 就别想过关 */

/* C 里 sizeof('a') 往往是 sizeof(int)；C++ 里字符字面量就是 char */
```

`class`、`virtual`、`new` 在 C++ 里是关键字。C 源码里如果有变量叫 `class`，改成 `.cpp` 会直接语法错误——这就是「把 `.c` 改名再编译」的意义：缝会自己跳出来。

### 本质

**C++ 在语言核心层把「类型」当成安全网，而不是建议。**

- **何时用到：** 每次把 C 代码当 C++ 编、每次看到 `static_cast` / 链接报错「未声明」。
- **系统位置：** 这是 C++ 相对 C 的第一道分水岭，后面的引用、重载、模板，全都建立在「类型必须说清楚」之上。
- **嵌入式提醒：** 和硬件打交道时仍可能要 `reinterpret_cast` 把整数地址变成寄存器指针。那是明确承认「我在做危险的事」，不是退回 C 的悄悄转换。

---

## 知识点 2：引用（reference）——给变量起外号

### 先想这个问题

C 里改别人的对象，你只有一种正规手段：传指针。指针很能干，也能干坏事：可以为空、可以指飞、可以中途改去指别人。

引用的发明动机很土：很多时候你只是想说「这就是那个对象，别跟我玩空指针」。

把它想成：指针是门牌号（可以写错、可以空白）；引用是外号（外号必须一开始就绑在真人身上，而且不能改口去叫别人）。

### 它长什么样

```cpp
void bump(int &x)
{
    ++x;                 /* 改的就是调用方那个变量本身 */
}

void bump_c_style(int *x)
{
    if (x == nullptr) {  /* 指针必须先防空 */
        return;
    }
    ++(*x);
}

int main()
{
    int speed = 0;
    bump(speed);         /* 看起来像传值，其实绑的是 speed 自己 */
    bump_c_style(&speed);
}
```

和 `const` 搭档时特别常见——你在 C 里已经用 `const` 防改，C++ 把这套用在引用上：

```cpp
/* 只读地绑住对象：不能改，也不拷贝大结构体 */
float read_error(const PID_t &pid)
{
    return pid.prev_error;
}
```

对照你的 `PID_update(PID_t *pid, ...)`：C++ 里更常写成 `PID_t &` 或 `const PID_t &`。不是指针消失了，是「必存在的别名」从指针里被拆了出来。

### 本质

**引用是编译期保证非空、不可重绑的别名；指针是运行期可以缺席的地址。**

- **何时用到：** 函数要改调用方的对象，或避免拷贝大结构，且你确信对象活着——用引用。对象可能不存在、要延迟绑定、要放进数组——继续用指针。
- **系统位置：** 语言核心的「名字绑定」机制。它不是新存储，不占「另一个对象」。后面的运算符重载、范围 for、很多标准库接口，默认都按引用说话。
- **读代码口诀：** 看到 `T &` 想「就是那个 T」；看到 `T *` 想「也许有一个 T」。

---

## 知识点 3：函数重载（overload）——同名不同工

### 先想这个问题

C 里名字是全球唯一身份证：`print_int`、`print_float`、`print_str`。你在状态机里已经写过 `LcdMotionFsm_Init` / `LcdMotionFsm_Tick` 这种前缀，就是在用人肉命名空间对抗重名。

C++ 允许同一个函数名，靠参数类型把人分开。打印不必叫三兄弟，都叫 `print` 就行——编译器看你塞进去的是 `int` 还是 `const char *`，自己去对号入座。

### 它长什么样

```cpp
#include <cstdio>

void log_val(int x)
{
    std::printf("i=%d\n", x);
}

void log_val(float x)
{
    std::printf("f=%f\n", x);
}

void log_val(const char *s)
{
    std::printf("s=%s\n", s);
}

void demo()
{
    log_val(3);      /* 走 int 版本 */
    log_val(3.0f);   /* 走 float 版本 */
    log_val("peel"); /* 走字符串版本 */
}
```

这就是为什么 C++ 要 **name mangling**（名字修饰）：链到目标文件里时，三个 `log_val` 其实叫三个不同的符号。第 4 周混编会再碰到这个整容手术。

默认参数是重载的便宜亲戚：

```cpp
void tick(float dt, bool verbose = false);  /* 可写 tick(0.01f) */
```

### 本质

**重载是编译期按参数类型选函数，不是运行期变身。**

- **何时用到：** 同一件语义、多种数据类型（打印、构造、加减）。不是为了少打几个字符而把无关操作塞进一个名字。
- **系统位置：** 属于「静态多态」（static polymorphism）——编译期就能定下来调谁。虚函数那套运行期查表是另一路，第 3 周再认。
- **和宏的关系：** 宏没有类型，`min_t(type, x, y)` 要你自己把类型喂进去。重载让类型自己说话。

---

## 知识点 4：命名空间（namespace）——给标识符分区

### 先想这个问题

C 的全局函数像全校共用一个花名册：两个模块都想叫 `init`，只能加前缀 `Pid_Init`、`LcdMotionFsm_Init`。你仓库里已经在用人肉前缀做这件事。

`namespace` 就是语言把前缀升级成「小区门牌」。小区里可以都叫 `init`，出门才报全名 `pid::init`。

### 它长什么样

```cpp
namespace pid {
struct Controller {
    float kp, ki, kd;
};

void init(Controller &c, float kp, float ki, float kd);
}

namespace lcd {
enum class State { Idle, Exposure, Peel };
void init();
}

void boot()
{
    pid::Controller c{};
    pid::init(c, 1.5f, 6.0f, 0.0f);
    lcd::init();
}
```

标准库全部住在 `std` 里，所以你会看到 `std::printf`、`std::vector`。头文件里不要 `using namespace std;`，否则等于拆掉小区围墙，污染再次发生。

### 本质

**namespace 是编译期的名字隔离，不产生运行时代价。**

- **何时用到：** 库、模块、避免和别人的 `max` / `init` 撞车。读代码时 `std::` 就是在说「这是标准库的」。
- **系统位置：** 和头文件防护、`static` 内部链接是同一类问题——「名字归谁」。C 用前缀和 `static`，C++ 多了 namespace 这一层正规分区。
- **和 class 的差别：** class 隔离的是数据和操作；namespace 只隔离名字，里面可以是函数、类型、常量，不必属于同一个对象。

---

## 知识点 5：`constexpr` ——有类型的编译期常量

### 先想这个问题

你在 `extend_syntax.c` 里已经给宏交过学费：少一层括号，`3 * add(1, 2)` 就变成 `3 * 1 + 2`。宏是预处理器的复印机，贴上去之前编译器根本不认识它是什么类型。

`constexpr` 想解决的是：我要一个**编译期就能定下来的值**，但它必须是真正的语言对象——有类型、能进调试器、不会在展开时改写优先级。

### 它长什么样

```cpp
/* C 习惯 */
#define SETPOINT 100

/* C++ 更稳的对应 */
constexpr int kSetpoint = 100;

constexpr float clampf(float x, float lo, float hi)
{
    return x < lo ? lo : (x > hi ? hi : x);
}

float demo(float u)
{
    /* 限幅边界在编译期就是常量，函数本身也可在编译期求值 */
    return clampf(u, -kSetpoint, kSetpoint);
}
```

`decltype` 则是你用过的 GNU `typeof` 的标准版：问编译器「这东西是什么类型」，而不是问它的值。

```cpp
int layer = 0;
decltype(layer) next = layer;  /* next 与 layer 同类型 */
```

### 本质

**`constexpr` 把「编译期可计算」收进类型系统；宏把文本贴进源码。**

- **何时用到：** 数组大小、限幅、魔数、短小纯函数。表达式会变、依赖运行期输入时，老老实实写普通函数。
- **系统位置：** 位于「编译期计算」这条轴的入门处。再往上是模板、C++20 的 `consteval`，那些本轮不碰。
- **对你而言的替换表：** `#define MAX` → `constexpr`；`typeof` → `decltype` / `auto`；带副作用的语句表达式宏 → `inline` 函数（第 3 周模板会再升级一档）。

---

## 本周收工

把任意一个小 `.c` 改名为 `.cpp`，用 `g++ -std=c++17` 编译。把报错记下来：每一条缝都对应上面某一个知识点。

能用自己的话回答这句，就算过关：

> C 的 `void *` 为什么在 C++ 里不能悄悄变成 `int *`？
