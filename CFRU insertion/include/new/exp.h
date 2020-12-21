#pragma once

#include "../global.h"
#include "../pokemon.h"

/**
 * \file exp.h
 * \brief Contains functions relating to Pokemon gaining experience in battle.
 *		  Also contains the function repsonsible for assigning Pokemon Effort
 *		  Values after battle.
 */

//Exported Functions
bool8 AddEVs(struct Pokemon* mon, u8 statId, u16 numToAdd);

//Functions Hooked In
void atk23_getexp(void);
void PlayerHandleExpBarUpdate(void);
