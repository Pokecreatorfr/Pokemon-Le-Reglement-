#ifndef GUARD_BATTLE_DOME_CARDS_H
#define GUARD_BATTLE_DOME_CARDS_H

#include "global.h"

bool16 __attribute__((long_call)) dp13_810BB8C(void);
u16 __attribute__((long_call)) sub_818D3E4(u16 species, u32 otId, u32 personality, u8 flags, s16 x, s16 y, u8 paletteSlot, u16 paletteTag);
u16 __attribute__((long_call)) sub_818D7D8(u16 species, u32 otId, u32 personality, bool8 isFrontPic, s16 x, s16 y, u8 paletteSlot, u16 paletteTag);
u16 __attribute__((long_call)) sub_818D820(u16 spriteId);
u16 __attribute__((long_call)) sub_818D8AC(u16 species, bool8 isFrontPic, s16 x, s16 y, u8 paletteSlot, u16 paletteTag);
u16 __attribute__((long_call)) sub_818D8F0(u16 spriteId);
u16 __attribute__((long_call)) sub_818D938(u16 species, bool8 isFrontPic, u16 destX, u16 destY, u8 paletteSlot, u8 windowId);
u8 __attribute__((long_call)) sub_818D97C(u8 a0, u8 a1);
u8 __attribute__((long_call)) sub_810C2A4(u16, u8, s16, s16, u8, u16);
void __attribute__((long_call)) sub_810C2E8(u16);

#endif //GUARD_BATTLE_DOME_CARDS_H
