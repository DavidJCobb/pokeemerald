#ifndef GUARD_GLOBAL_BERRY_H
#define GUARD_GLOBAL_BERRY_H

#define BERRY_NAME_LENGTH 6
#define BERRY_ITEM_EFFECT_COUNT 18

struct Berry
{
    const u8 name[BERRY_NAME_LENGTH + 1];
    u8 firmness;
    u16 size;
    u8 maxYield;
    u8 minYield;
    const u8 *description1;
    const u8 *description2;
    u8 stageDuration;
    u8 spicy;
    u8 dry;
    u8 sweet;
    u8 bitter;
    u8 sour;
    u8 smoothness;
};

// with no const fields

struct Berry2
{
#include "lu/generated/struct-members/Berry2.members.h"
};

struct EnigmaBerry
{
#include "lu/generated/struct-members/EnigmaBerry.members.h"
};

struct BattleEnigmaBerry
{
    /*0x00*/ u8 name[BERRY_NAME_LENGTH + 1];
    /*0x07*/ u8 holdEffect;
    /*0x08*/ u8 itemEffect[BERRY_ITEM_EFFECT_COUNT];
    /*0x1A*/ u8 holdEffectParam;
};

struct BerryTree
{
#include "lu/generated/struct-members/BerryTree.members.h"
};

#endif // GUARD_GLOBAL_BERRY_H
