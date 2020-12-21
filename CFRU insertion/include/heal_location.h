#ifndef GUARD_HEAL_LOCATION_H
#define GUARD_HEAL_LOCATION_H

#include "global.h"

// Exported type declarations

struct UnkStruct_0859F534 {
    const union AffineAnimCmd *const *const affineAnims;
    void (* callback)(struct Sprite *sprite);
};

struct HealLocation
{
    s8 group;
    s8 map;
    u16 x;
    u16 y;
};

// Exported RAM declarations

// Exported ROM declarations

const struct HealLocation *GetHealLocationPointer(u32 loc);

#endif //GUARD_HEAL_LOCATION_H
