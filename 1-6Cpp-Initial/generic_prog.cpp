#include <cstdio>
#include <cstring>
#include <vector>

/* 函数模板：按调用处的类型复印一份函数，类型仍受检查 */
template <typename T>
T max_of(T a, T b)
{
    return a > b ? a : b;
}

/* 两个类型参数：允许左右操作数不是同一种类型 */
template <typename T, typename U>
auto add_of(T a, U b) -> decltype(a + b)
{
    return a + b;
}

/* 全特化：某一类型不走通用比较，避免指针比的是地址 */
template <>
const char *max_of<const char *>(const char *a, const char *b)
{
    return std::strcmp(a, b) > 0 ? a : b;
}

/* 类模板：数据与操作一起按元素类型参数化 */
/*▲注意 结构体 默认是public的class， 含义比C中的丰富*/
template <typename T>
struct Vec2 {
    T x;
    T y;

    T mag2() const
    {
        return x * x + y * y;
    }
};

/* 非类型参数：值也可以写进模板，编译期成为类型的一部分 */
template <typename T, int N>
struct Ring {
    T buf[N];
    int head = 0;

    void push(T v)
    {
        buf[head] = v;
        head = (head + 1) % N;
    }

    T last() const
    {
        int i = (head + N - 1) % N;
        return buf[i];
    }
};

int main()
{
    /* 实参推导：编译器生成 max_of<int> */
    auto mi = max_of(3, 4);

    /* 显式写出模板实参，读代码时尖括号就是「什么的什么」 */
    auto mf = max_of<float>(1.5f, 0.5f);

    /* 走 const char* 特化，比较的是字符串内容 ，相当于被特殊化处理了不走通用的比较方法*/
    auto ms = max_of("peel", "home");

    auto sum = add_of(1, 2.5f);

    Vec2<float> v{3.0f, 4.0f};
    Ring<int, 4> ring;
    ring.push(10);
    ring.push(20);

    /* 标准库容器本身就是类模板的实例 */
    std::vector<float> samples;
    samples.push_back(1.0f);
    samples.push_back(2.0f);

    std::printf("max_i=%d max_f=%f max_s=%s sum=%f mag2=%f last=%d\n",
                mi, mf, ms, sum, v.mag2(), ring.last());

    for (auto x : samples) {
        std::printf("sample=%f\n", x);
    }

    return 0;
}
