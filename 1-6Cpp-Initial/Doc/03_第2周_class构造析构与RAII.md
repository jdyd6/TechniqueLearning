# 第 2 周：class、构造析构、RAII

> 本周停在：能说出 RAII 和 `malloc` / `free` 配对相比，到底防的是哪类 bug。
>
> 对照仓库：`1-1PID_Initial` 的 `PID_t` + `PID_init` / `PID_update`；`1-5Extended/property.c` 里的 GNU `constructor` / `destructor` 属性。
>
> 这是四周里**唯一必须换脑子**的一周。其它周大多是认语法，这周是换资源观。

---

## 知识点 1：`class` ——把抽屉和说明书订成一本手册

### 先想这个问题

你现在的 PID 是这样拆开的：

```c
typedef struct {
    float kp, ki, kd;
    float integral, prev_error;
    float dt;
} PID_t;

void  PID_init(PID_t *pid, float kp, float ki, float kd, float dt);
float PID_update(PID_t *pid, float setpoint, float measured_value);
```

`PID_t` 是抽屉，函数是散落在房间各处的说明书。谁拿到抽屉都可以直接改 `integral`，也可以忘记调用 `PID_init` 就拿去 `update`。语言不管，全靠你记。

`class` 想干的事很不客气：把数据和允许的操作订死在一起，并且规定哪些抽屉隔层外人不许伸手。

### 它长什么样

```cpp
class PidController {
public:
    /* 出生时就把增益和采样周期绑死 */
    PidController(float kp, float ki, float kd, float dt);

    /* 推进一步，返回控制量 */
    float update(float setpoint, float measured);

    /* 清积分，对应原来可能散落的 reset */
    void reset();

private:
    float kp_;
    float ki_;
    float kd_;
    float dt_;
    float integral_;
    float prev_error_;
};
```

`struct` 在 C++ 里并没有消失。差别几乎只有默认访问权：

- `struct`：默认 `public`，适合「就是一包数据」——比如你的 `LcdPrintProfile_t`。
- `class`：默认 `private`，适合「有不变量要守」——比如 PID 的积分不能被外面随便改。

C++ Core Guidelines 的说法就是：纯数据用 `struct`，要守规矩的用 `class`。

调用看起来像「对象自己会做事」，不再把 ctx 指针传来传去：

```cpp
PidController pid(1.5f, 6.0f, 0.0f, 0.01f);
float u = pid.update(100.0f, measured);
```

成员函数里有一个隐式的 `this`，就是 C 里你手写的那个 `PID_t *pid`。C++ 只是把它藏进调用语法里。

### 本质

**class 是「数据 + 合法操作 + 访问边界」的编译期契约，不是运行时魔法。**

- **何时用到：** 一组字段必须一起初始化、一起使用、不许外面乱改内部状态。状态机上下文、PID、传感器滤波，都是典型候选人。
- **系统位置：** C++ 面向对象（OOP，Object-Oriented Programming）的地基。后面的构造/析构、继承、虚函数，全部长在 class 这棵树上。
- **何时不必上 class：** 真的只是传一包参数（层高、速度、曝光时间），`struct` 就很好。别为了「看起来更 C++」把纯数据包进一层空壳。

---

## 知识点 2：构造函数与析构函数 ——对象的出生证明和死亡证明

### 先想这个问题

你在 `property.c` 里写过：

```c
static void __attribute__((constructor)) boot(void);
static void __attribute__((destructor)) shutdown(void);
```

那是 **进程级** 钩子：程序一加载就 `boot`，`main` 结束再 `shutdown`。全进程只有这一出戏。

C++ 的构造函数（constructor）/ 析构函数（destructor）是 **每个对象自己的** 出生证明和死亡证明。你造 10 个 PID，就会跑 10 次构造；每个对象离开作用域，就跑一次析构。

GNU 属性像公司年会开幕闭幕；C++ 构造/析构像每张工牌的入职和离职。别混。

### 它长什么样

```cpp
class PidController {
public:
    PidController(float kp, float ki, float kd, float dt)
        : kp_(kp)
        , ki_(ki)
        , kd_(kd)
        , dt_(dt)
        , integral_(0.0f)
        , prev_error_(0.0f)
    {
        /* 成员已经在初始化列表里就位，函数体通常很短 */
    }

    ~PidController()
    {
        /* 这个 PID 没有堆资源，析构可以为空，甚至不写 */
    }

private:
    float kp_, ki_, kd_, dt_;
    float integral_;
    float prev_error_;
};
```

冒号后面那一串叫 **成员初始化列表**（member initializer list）。它不是赋值秀，是「成员出生时的值」。引用成员、`const` 成员只能走这条路，因为它们不允许先默认构造再改。

对比 C：

```c
PID_t pid;                         /* 出生时字段是垃圾值 */
PID_init(&pid, 1.5f, 6.0f, 0.0f, 0.01f);  /* 全靠你记得调用 */
```

C++ 里构造函数跑完，对象才算存在。你没法合法地拿到一具「没 init 过的 PID」。

析构函数在对象寿命结束时自动调用：局部变量离开 `}`、临时对象用完、异常把栈展开（stack unwinding）时也会走。这就是下一节 RAII 能成立的机械基础。

### 本质

**构造保证「存在即合法」；析构保证「消失即清理」。GNU constructor 属性管进程，不管对象。**

- **何时用到：** 几乎每个 class。有资源（文件、内存、锁、socket）时析构尤其重要；没有资源也可以只写构造，把字段一次设对。
- **系统位置：** class 生命周期的两端。它是 RAII 的发动机——没有自动析构，RAII 就是一句空话。
- **读代码时：** 先看构造函数参数，那就是「这个对象出生需要什么」。这比先看成员函数更值钱。

---

## 知识点 3：RAII ——门锁装了弹簧

### 先想这个问题

C 的资源纪律是社交约定：`malloc` 了就要 `free`，`fopen` 了就要 `fclose`。你在柔性数组例子里写得很规范：

```c
struct Room_area *p = malloc(sizeof(*p) + extra);
/* ... 使用 ... */
free(p);
```

一旦中间 `return`、一旦有人加了 `goto`、一旦以后 C++ 代码丢出异常——那行 `free` 就可能没人跑到。钥匙还在你口袋里，门却敞着。

RAII（Resource Acquisition Is Initialization，资源获取即初始化）的土办法是：别让人记着还钥匙，把弹簧锁装上门——人一离开房间，门自己锁。

「人离开」在 C++ 里就是对象析构。「锁门」就是析构函数里的 `free` / `fclose` / `delete`。

### 它长什么样

先看手写版，把机制看光：

```cpp
class HeapBuffer {
public:
    explicit HeapBuffer(std::size_t n)
        : data_(static_cast<char *>(std::malloc(n)))
        , size_(n)
    {
    }

    ~HeapBuffer()
    {
        std::free(data_);   /* 离开作用域必走这里 */
    }

    char *data() { return data_; }

private:
    char *data_;
    std::size_t size_;
};

void parse_line()
{
    HeapBuffer buf(64);
    /* 就算中途 return，buf 的析构仍会 free */
}
```

第 4 周会看到标准库已经帮你写好了这类弹簧锁：`std::vector`、`std::string`、`std::unique_ptr`。你现在只要记住原则，不必先背那三个名字。

RAII 防的不是「忘记 malloc」，而是：

1. 忘记 `free`
2. 有多条退出路径，只在一条上 `free`
3. 异常把控制流扯走，清理代码没机会跑
4. 拷贝对象导致同一块内存被 `free` 两次（这要靠下一层：禁止拷贝或实现拷贝语义；本轮知道「有这坑」即可）

### 本质

**RAII 把资源寿命焊在对象寿命上：对象活着资源就在，对象死了资源就还。**

- **何时用到：** 凡是「获取之后必须释放」的东西——堆内存、文件、锁、socket、DMA 缓冲的映射。嵌入式里如果全程静态分配、根本没有堆，你可以不用堆上的 RAII，但「外设句柄包进对象、析构时关外设」仍然是同一思想。
- **系统位置：** 现代 C++ 资源管理的中轴。智能指针、容器、fstream，全是 RAII 的实例。不会 RAII，就还在用 C 的社交约定写 C++。
- **和 C 对照：** C 把清理写成你要执行的语句；C++ 把清理写成类型的属性。语句可能被跳过，类型的析构在寿命结束时由语言保证执行。

---

## 知识点 4：访问控制 ——不变量靠门禁守，不靠口头提醒

### 先想这个问题

`LcdMotionFsm_t` 里什么都能改：外面可以直接把 `state` 写成 `LCD_STATE_FINISH`，绕过 `Tick`。C 里这叫「结构体就是公开内存布局」。

C++ 用 `public` / `private` 把「调用方该看的」和「内部账本」切开。不是为了神秘，是为了让非法状态更难形成。

### 它长什么样

```cpp
class LcdFsm {
public:
    void start();
    void tick(float dt);
    bool is_done() const;   /* 尾部 const：承诺不改成员 */

private:
    /* 状态只能由 tick / start 改，外面伸手进来会编译失败 */
    enum class State { Idle, Exposure, Peel, Finish };
    State state_ = State::Idle;
    float timer_ = 0.0f;
};
```

成员函数后面的 `const`，是 C 里 `const PID_t *` 的面向对象说法：`this` 变成指向常量的指针，函数保证不改对象。

### 本质

**访问控制是编译期门禁，用来保护不变量，不是用来藏实现炫技。**

- **何时用到：** 字段之间有约束（积分要限幅、状态只能按边转移、句柄不能为空）。没有约束的纯数据包，保持 `public` 更诚实。
- **系统位置：** class 的边界定义。没有边界，构造函数保证的「存在即合法」下一秒就会被外面改坏。

---

## 本周收工

在脑子里（或另开一个小 `.cpp`）把 `PID_t` 收成 `PidController`。不必改原项目。体会三件事：

1. 调用处不再先声明再 `init`
2. `integral_` 被 `private` 挡住
3. 如果以后内部有堆缓冲，清理写在析构里，而不是每个 `return` 前

能用自己的话回答这句，就算过关：

> GNU 的 `constructor` 属性和 C++ 类的构造函数，分别在什么时刻跑？各管几份对象？
