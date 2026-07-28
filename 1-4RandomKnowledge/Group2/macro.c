#include <stdio.h>
#include "macro.h"


#define SECTION_1 1
#define SECTION_2 0
#define SECTION_3 0
#define SECTION_4 0
#define SECTION_5 0
#define SECTION_6 0
#define SECTION_7 0



/*梳理Marlin的宏嵌套*/
#if SECTION_1

  #define TERN(O,A,B)         _TERN(_ENA_1(O),B,A)    // OPTION converted to '0' or '1'
    #define _TERN(E,V...)       __TERN(_CAT(T_,E),V)    // Prepend 'T_' to get 'T_0' or 'T_1'
        #define __TERN(T,V...)      ___TERN(_CAT(_NO,T),V)  // Prepend '_NO' to get '_NOT_0' or '_NOT_1'
            #define ___TERN(P,V...)     THIRD(P,V)              // If first argument has a comma, A. Else B.
                #define THIRD(a,b,c,...) c  

  #define TERN0(O,A)          _TERN(_ENA_1(O),0,A)    // OPTION converted to A or '0'
  #define TERN1(O,A)          _TERN(_ENA_1(O),1,A)    // OPTION converted to A or '1'

  #define TERN_(O,A)          _TERN(_ENA_1(O),,A)     // OPTION converted to A or '<nul>'


  #define _ENA_1(O)           _ISENA(CAT(_IS,CAT(ENA_, O)))
    #define _ISENA(V...)        IS_PROBE(V)
          #define IS_PROBE(V...) SECOND(V, 0)     // Get the second item passed, or 0
              #define SECOND(a,b,...)  b

#define _CAT(a,V...) a##V

/*
#define _ISENA_SENSOR ~, 1
#define _NOT_1            1
#define _NOT_0            ~,0

梳理展开过程： 假设o 表示SENSOR, A 表示“有”，B 表示“无”，则：
TERN(SENSOR, "有", "无")
    _TERN(_ENA_1(SENSOR), "无", "有")
    _TERN(_ISENA(_CAT(_IS,CAT(ENA_, SENSOR))), "无", "有")
    _TERN(IS_PROBE(_ISENA_SENSOR), "无", "有" )
    _TERN(SECOND(_ISENA_SENSOR, 0), "无", "有" )
    _TERN(SECOND(~, 1, 0), "无", "有" )
    _TERN(1, "无", "有" )
        __TERN(T_1, "无", "有")
            ___TERN(_NOT_1, "无", "有")
                THIRD(1, "无", "有")
                    "有"
    




目的：在宏展开阶段实现 条件 ? A : B，让配置开关能渗进任意代码位置。
*/


#endif