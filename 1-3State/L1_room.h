#ifndef L1_ROOM_H
#define L1_ROOM_H

/* 单房间冒险 - L1 状态机 */

typedef enum {
    ST_ROOM_SAFE,       /* 安全，可四处查看 */
    ST_FIGHTING,        /* 与史莱姆战斗 */
    ST_OPENING_CHEST,   /* 正在开宝箱 */
    ST_DEAD             /* 死亡 */
} RoomState;

typedef enum {
    EV_LOOK_AROUND,     /* 环顾四周 */
    EV_ATTACK,          /* 攻击 */
    EV_OPEN_CHEST,      /* 开宝箱 */
    EV_HIT_BY_MONSTER,  /* 被怪物反击 */
} RoomEvent;

/* 房间上下文：状态 + 游戏数据 */
typedef struct {
    RoomState state;
    int       hp;       /* 玩家血量 */
    int       slime_hp; /* 史莱姆血量 */
    int       gold;     /* 金币 */
} RoomContext;

/* 处理一次事件，可能触发状态转移 */
void process_event(RoomContext *ctx, RoomEvent event);

#endif
