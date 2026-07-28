#include "L1_room.h"
#include <stdio.h>
#include <stdlib.h>

/* 史莱姆初始血量 */
#define SLIME_INIT_HP   5
/* 环顾：遇怪概率（%） */
#define ENCOUNTER_RATE  70
/* 开箱：出金币概率（%） */
#define CHEST_GOOD_RATE 50

static const char *state_name(RoomState st)
{
    switch (st) {
    case ST_ROOM_SAFE:      return "安全";
    case ST_FIGHTING:       return "战斗";
    case ST_OPENING_CHEST:  return "开箱";
    case ST_DEAD:           return "死亡";
    default:                return "未知";
    }
}

/* 史莱姆反击一次 */
static void slime_counter(RoomContext *ctx)
{
    ctx->hp -= 2;
    printf("史莱姆反击！你剩余 %d HP\n", ctx->hp);
    if (ctx->hp <= 0) {
        printf("你倒下了……\n");
        ctx->state = ST_DEAD;
    }
}

/* 结算宝箱内容，并回到安全或死亡 */
static void resolve_chest(RoomContext *ctx)
{
    if (rand() % 100 < CHEST_GOOD_RATE) {
        ctx->gold += 10;
        printf("宝箱里是 10 金币！当前共 %d 金\n", ctx->gold);
    } else {
        ctx->hp -= 5;
        printf("陷阱！宝箱喷出毒针，扣 5 HP，剩余 %d HP\n", ctx->hp);
    }

    if (ctx->hp <= 0) {
        printf("你倒下了……\n");
        ctx->state = ST_DEAD;
    } else {
        ctx->state = ST_ROOM_SAFE;
    }
}

/* 内部事件处理，不含状态行打印（供递归调用） */
static void room_apply(RoomContext *ctx, RoomEvent event)
{
    switch (ctx->state) {

    case ST_ROOM_SAFE:
        switch (event) {
        case EV_LOOK_AROUND:
            if (rand() % 100 < ENCOUNTER_RATE) {
                ctx->slime_hp = SLIME_INIT_HP;
                ctx->state = ST_FIGHTING;
                printf("一只史莱姆跳了出来！（史莱姆 HP=%d）\n", ctx->slime_hp);
            } else {
                ctx->gold += 5;
                printf("角落发现 5 金币，当前共 %d 金\n", ctx->gold);
            }
            break;

        case EV_OPEN_CHEST:
            ctx->state = ST_OPENING_CHEST;
            resolve_chest(ctx);
            break;

        case EV_ATTACK:
        case EV_HIT_BY_MONSTER:
            printf("周围没有敌人，无法攻击。\n");
            break;

        default:
            break;
        }
        break;

    case ST_FIGHTING:
        switch (event) {
        case EV_ATTACK:
            ctx->slime_hp -= 3;
            printf("你砍向史莱姆，它剩余 %d HP\n", ctx->slime_hp);

            if (ctx->slime_hp <= 0) {
                printf("史莱姆化为了一滩黏液，战斗结束。\n");
                ctx->state = ST_ROOM_SAFE;
            } else {
                slime_counter(ctx);
            }
            break;

        case EV_HIT_BY_MONSTER:
            slime_counter(ctx);
            break;

        case EV_LOOK_AROUND:
        case EV_OPEN_CHEST:
            printf("战斗中，没空做别的！\n");
            break;

        default:
            break;
        }
        break;

    case ST_OPENING_CHEST:
        if (event == EV_OPEN_CHEST)
            resolve_chest(ctx);
        break;

    case ST_DEAD:
        printf("你已经死了，无法行动。\n");
        break;

    default:
        printf("无效状态。\n");
        break;
    }
}

void process_event(RoomContext *ctx, RoomEvent event)
{
    room_apply(ctx, event);
    printf("  -> 当前状态：%s | HP=%d | 金=%d\n",
           state_name(ctx->state), ctx->hp, ctx->gold);
}
