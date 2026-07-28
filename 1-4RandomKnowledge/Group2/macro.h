#ifndef __MACRO_H__
#define __MACRO_H__

/*
 * 教学用“穷人版”TERN：选项必须是 0 或 1
 * 用法：TERN(开关, 开时结果, 关时结果)
 * 不必抠 Marlin 探针链，接口语义一样。
 */

/* 先展开再拼接，否则 ## 会粘住未展开的名字 */
#define _PRIMITIVE_CAT(a, b) a##b
#define _CAT(a, b)           _PRIMITIVE_CAT(a, b)

#define _TERN_1(A, B) A
#define _TERN_0(A, B) B

/* TERN(O,A,B)：O 为 1 → A，为 0 → B */
#define TERN(O, A, B) _CAT(_TERN_, O)(A, B)
#define TERN0(O, A)   TERN(O, A, 0)
#define TERN1(O, A)   TERN(O, A, 1)

#endif
