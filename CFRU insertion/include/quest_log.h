#ifndef GUARD_QUEST_LOG_H
#define GUARD_QUEST_LOG_H

#include "global.h"

struct UnkStruct_3005E90
{
    u8 unk_0;
    u8 unk_1_0:1;
    u8 unk_1_1:1;
    u8 unk_1_2:1;
    u8 unk_1_3:1;
    u8 unk_1_4:1;
    u8 unk_2;
};

extern u8 gUnknown_3005E88;
extern u16 gUnknown_3005E8C;
extern struct UnkStruct_3005E90 gUnknown_3005E90;
extern struct UnkStruct_203AE98 * gUnknown_3005E94;

void sub_8112720(u8);
void sub_8113550(u16, const u16 *);
void __attribute__((long_call)) WriteQuestLog(u8 a1, u8 a2, u16 a3, u16 a4);

#endif //GUARD_QUEST_LOG_H
