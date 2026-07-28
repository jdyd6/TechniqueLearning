#include "L1_room.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    RoomContext ctx = {
        .state    = ST_ROOM_SAFE,
        .hp       = 10,
        .slime_hp = 0,
        .gold     = 0,
    };

    /* 固定种子可复现；改成 time(NULL) 则每次随机 */
    srand(60);

    /* 模拟一段冒险脚本（seed=42 时：先捡金→开箱→遇怪→战斗） */
    RoomEvent script[] = {
        EV_LOOK_AROUND,
        EV_ATTACK,
        EV_ATTACK,
        EV_OPEN_CHEST,
        EV_LOOK_AROUND,   /* 第二次环顾，触发战斗 */
        EV_ATTACK,
        EV_ATTACK,
    };

    int n = (int)(sizeof(script) / sizeof(script[0]));

    printf("=== L1 单房间冒险 ===\n");
    printf("初始 HP=%d\n\n", ctx.hp);

    for (int i = 0; i < n; i++) {
        printf("[事件 %d] ", i + 1);
        process_event(&ctx, script[i]);
        printf("\n");

        if (ctx.state == ST_DEAD)
            break;
    }

    printf("=== 冒险结束 ===\n");
    printf("最终 HP=%d，金币=%d，状态=%d\n", ctx.hp, ctx.gold, ctx.state);
    return 0;
}
