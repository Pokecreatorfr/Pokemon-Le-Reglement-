#include "../defines.h"
#include "../defines_battle.h"
#include "../../include/random.h"
#include "../../include/constants/items.h"

#include "../../include/new/ability_tables.h"
#include "../../include/new/accuracy_calc.h"
#include "../../include/new/ai_advanced.h"
#include "../../include/new/ai_util.h"
#include "../../include/new/ai_master.h"
#include "../../include/new/ai_scripts.h"
#include "../../include/new/battle_start_turn_start.h"
#include "../../include/new/battle_util.h"
#include "../../include/new/damage_calc.h"
#include "../../include/new/frontier.h"
#include "../../include/new/general_bs_commands.h"
#include "../../include/new/util.h"
#include "../../include/new/item.h"
#include "../../include/new/move_tables.h"
/*
ai_positives.c
	all possible additions to an AIs move viability
*/

extern const struct NaturalGiftStruct gNaturalGiftTable[];
extern const struct FlingStruct gFlingTable[];

#define TARGETING_PARTNER (bankDef == data->bankAtkPartner)

//Doubles is now defined as being a non 1v1 Double Battle
#undef IS_DOUBLE_BATTLE
#define IS_DOUBLE_BATTLE (gBattleTypeFlags & BATTLE_TYPE_DOUBLE && ((BATTLER_ALIVE(data->foe1) && BATTLER_ALIVE(data->foe2)) || BATTLER_ALIVE(data->bankAtkPartner)))

u8 AIScript_Positives(const u8 bankAtk, const u8 bankDef, const u16 originalMove, const u8 originalViability, struct AIScript* data)
{
	u32 i, j;
	u16 predictedMove = IsValidMovePrediction(bankDef, bankAtk); //The move the target is likely to make against the attacker
	u8 class = GetBankFightingStyle(bankAtk);
	s16 viability = originalViability;

	//Get relevant params
	u16 move = TryReplaceMoveWithZMove(bankAtk, bankDef, originalMove);

	u8 atkAbility = GetAIAbility(bankAtk, bankDef, move);
	u8 defAbility = GetAIAbility(bankDef, bankAtk, predictedMove);

	if (!NO_MOLD_BREAKERS(atkAbility, move)
	&& gMoldBreakerIgnoredAbilities[defAbility])
		defAbility = ABILITY_NONE;

	u8 moveEffect = gBattleMoves[move].effect;
	u8 moveType = GetMoveTypeSpecial(bankAtk, move);
	u8 moveSplit = CalcMoveSplit(bankAtk, move);

	//Begin
	if (IS_DOUBLE_BATTLE && TARGETING_PARTNER)
		return AIScript_Partner(bankAtk, data->bankAtkPartner, originalMove, originalViability, data);

	if (IsAnyMaxMove(move))
		moveEffect = GetAIMoveEffectForMaxMove(move, bankAtk, bankDef);

	switch (moveEffect) {
		case EFFECT_HIT:
			// to do
			break;

		case EFFECT_SLEEP:
		case EFFECT_YAWN:
		AI_SLEEP_CHECKS:
			IncreaseSleepViability(&viability, class, bankAtk, bankDef, move);
			break;

		case EFFECT_ABSORB:
		case EFFECT_DREAM_EATER:
		AI_DRAIN_HP_CHECK: ;
			if (ShouldRecover(bankAtk, bankDef, move))
			{
				if (IsClassStall(class))
					INCREASE_VIABILITY(6);
				else if (IsClassDoublesTrickRoomSetup(class))
					INCREASE_VIABILITY(16);
				else if (IS_SINGLE_BATTLE)
					INCREASE_VIABILITY(3); //Past strongest move
				else
					IncreaseDoublesDamageViabilityToScore(&viability, class, 5, bankAtk, bankDef);
			}
			break;

		case EFFECT_PARALYZE_HIT:
			if (CalcSecondaryEffectChance(bankAtk, move) >= 75)
				goto AI_PARALYZE_CHECKS;
			break;

		case EFFECT_BURN_HIT:
			if (CalcSecondaryEffectChance(bankAtk, move) >= 75)
				goto AI_BURN_CHECKS;
			break;

		case EFFECT_POISON_HIT:
		case EFFECT_BAD_POISON_HIT:
			if (CalcSecondaryEffectChance(bankAtk, move) >= 75)
				goto AI_POISON_CHECKS;
			break;

		case EFFECT_EXPLOSION:
			if (predictedMove != MOVE_NONE //If foe isn't going to attack, don't kill yourself now
			&&  gBattleMoves[predictedMove].effect != EFFECT_PROTECT)
			{
				if (data->atkItemEffect == ITEM_EFFECT_GEM
				&& data->atkItemQuality == moveType) //AI Was meant to explode
				{
					INCREASE_VIABILITY(4); //Counteract the negative check
				}
				else if (data->atkItemEffect == ITEM_EFFECT_CUSTAP_BERRY
				&& GetHealthPercentage(bankAtk) <= 25)
				{
					INCREASE_VIABILITY(4); //Counteract the negative check
				}
			}
			break;

		case EFFECT_MIRROR_MOVE: //May cause issues with priority calcs?
			switch (move) {
				case MOVE_COPYCAT:
					if (MoveWouldHitFirst(move, bankAtk, bankDef))
					{
					COPYCAT_CHECK_LAST_MOVE:
						if (gNewBS->LastUsedMove != MOVE_NONE
						&& gNewBS->LastUsedMove != 0xFFFF
						&& !CheckTableForMove(gNewBS->LastUsedMove, gCopycatBannedMoves)
						&& !MoveInMoveset(gNewBS->LastUsedMove, bankAtk)) //If you have the move, use it directly
							return AIScript_Positives(bankAtk, bankDef, gNewBS->LastUsedMove, originalViability, data);
					}
					else
					{
						if (predictedMove == MOVE_NONE)
							goto COPYCAT_CHECK_LAST_MOVE;
						else if (!CheckTableForMove(predictedMove, gCopycatBannedMoves)
							 &&  !MoveInMoveset(predictedMove, bankAtk))
						{
							return AIScript_Positives(bankAtk, bankDef, predictedMove, originalViability, data);
						}
					}
					break;

				default: //Mirror Move
					if (gBattleStruct->lastTakenMoveFrom[bankAtk][bankDef] != MOVE_NONE)
						return AIScript_Positives(bankAtk, bankDef, gBattleStruct->lastTakenMoveFrom[bankAtk][bankDef], originalViability, data);
			}
			break;

		//Increased stat effects
		case EFFECT_ATTACK_UP:
		case EFFECT_ATTACK_UP_2:
			switch (move) {
				case MOVE_HONECLAWS:
					if (IsMovePredictionPhazingMove(bankDef, bankAtk))
						break;

					if (STAT_STAGE(bankAtk,STAT_STAGE_ATK) >= 8)
						goto AI_ACCURACY_PLUS;
					break;

				default:
				AI_ATTACK_PLUS:
					if (IsMovePredictionPhazingMove(bankDef, bankAtk))
						break;

					if (PhysicalMoveInMoveset(bankAtk) && atkAbility != ABILITY_CONTRARY)
						INCREASE_STAT_VIABILITY(STAT_STAGE_ATK, 8, 2);
					/*else if (IsMaxMove(move) && IS_DOUBLE_BATTLE && BATTLER_ALIVE(data->bankAtkPartner)
						&& PhysicalMoveInMoveset(data->bankAtkPartner) && atkAbility != ABILITY_CONTRARY)
					{
						IncreaseStatViability(&viability, class, 2, data->bankAtkPartner, bankDef, move, STAT_STAGE_ATK, 8);
					}*/
					break;
			}
			break;

		case EFFECT_DEFENSE_UP:
		case EFFECT_DEFENSE_UP_2:
		AI_DEFENSE_PLUS:
			if (IsMovePredictionPhazingMove(bankDef, bankAtk))
				break;

			/*
			switch (move) {
				case MOVE_FLOWERSHIELD:
					if (IsClassCleric(class)
					&& IsOfType(data->bankAtkPartner, TYPE_GRASS)
					&& data->atkPartnerAbility != ABILITY_CONTRARY
					&& STAT_STAGE(data->bankAtkPartner, STAT_STAGE_DEF) < 12
					&& BATTLER_ALIVE(data->bankAtkPartner))
					{
						viability += 5;
					}
					break;

				case MOVE_MAGNETICFLUX:
					if (IsClassCleric(class)
					&& (data->atkPartnerAbility == ABILITY_PLUS || data->atkPartnerAbility == ABILITY_MINUS)
					&& (STAT_STAGE(data->bankAtkPartner, STAT_STAGE_DEF) < 12 || STAT_STAGE(data->bankAtkPartner, STAT_STAGE_SPDEF) < 12))
					&& BATTLER_ALIVE(data->bankAtkPartner)
					{
						viability += 5;
					}
					else if (BankLikelyToUseMoveSplit(bankDef) == SPLIT_PHYSICAL)
						break;

					goto AI_SPECIAL_DEFENSE_PLUS;

				case MOVE_AROMATICMIST:
					if (IsClassCleric(class)
					&& STAT_STAGE(data->bankAtkPartner, STAT_STAGE_SPDEF) < 12
					&& BATTLER_ALIVE(data->bankAtkPartner)
					&& data->atkPartnerAbility != ABILITY_CONTRARY)
						viability += 5;
					break;
			}*/

			//Continue defense check
			if (BankLikelyToUseMoveSplit(bankDef, class) == SPLIT_PHYSICAL && atkAbility != ABILITY_CONTRARY)
				INCREASE_STAT_VIABILITY(STAT_STAGE_DEF, 10, 1);
			break;

		case EFFECT_SPEED_UP:
		case EFFECT_SPEED_UP_2:
		AI_SPEED_PLUS:
			if (IsMovePredictionPhazingMove(bankDef, bankAtk))
				break;
			if (atkAbility != ABILITY_CONTRARY)
				INCREASE_STAT_VIABILITY(STAT_STAGE_SPEED, 8, 3);
			break;

		case EFFECT_SPECIAL_ATTACK_UP:
		case EFFECT_SPECIAL_ATTACK_UP_2:
			if (IsMovePredictionPhazingMove(bankDef, bankAtk))
				break;
			switch (move)
			{
				case MOVE_GROWTH:
				case MOVE_WORKUP:
					if (PhysicalMoveInMoveset(bankAtk))
						goto AI_ATTACK_PLUS;
					else if (SpecialMoveInMoveset(bankAtk))
						goto AI_SPECIAL_ATTACK_PLUS;
					break;
				/*
				case MOVE_ROTOTILLER:
					if (IsOfType(data->bankAtkPartner,TYPE_GRASS))
						viability += 5;
					else if (PhysicalMoveInMoveset(bankAtk))
						goto AI_ATTACK_PLUS;
					else if (SpecialMoveInMoveset(bankAtk))
						break;
					goto FUNCTION_RETURN;

				case MOVE_GEARUP:
					if (data->atkPartnerAbility == ABILITY_PLUS || data->atkPartnerAbility == ABILITY_MINUS)
					{
						viability += 5;
						goto FUNCTION_RETURN;
					}
					else if (PhysicalMoveInMoveset(bankAtk))
						goto AI_ATTACK_PLUS;
					else if (SpecialMoveInMoveset(bankAtk))
						break;
					goto FUNCTION_RETURN;
				*/

				default:
				AI_SPECIAL_ATTACK_PLUS:
					if (IsMovePredictionPhazingMove(bankDef, bankAtk))
						break;
					if (SpecialMoveInMoveset(bankAtk) && atkAbility != ABILITY_CONTRARY)
						INCREASE_STAT_VIABILITY(STAT_STAGE_SPATK, 8, 2);
					break;
			}
			break;

		case EFFECT_SPECIAL_DEFENSE_UP:
		case EFFECT_SPECIAL_DEFENSE_UP_2:
		AI_SPECIAL_DEFENSE_PLUS: ;
			if (IsMovePredictionPhazingMove(bankDef, bankAtk))
				break;
			if (BankLikelyToUseMoveSplit(bankDef, class) == SPLIT_SPECIAL && atkAbility != ABILITY_CONTRARY)
				INCREASE_STAT_VIABILITY(STAT_STAGE_SPDEF, 10, 1);
			break;

		case EFFECT_ACCURACY_UP:
		case EFFECT_ACCURACY_UP_2:
		AI_ACCURACY_PLUS: ;
			if (IsMovePredictionPhazingMove(bankDef, bankAtk))
				break;
			if (MoveInMovesetWithAccuracyLessThan(bankAtk, bankDef, 90, TRUE) && defAbility != ABILITY_CONTRARY)
				INCREASE_STAT_VIABILITY(STAT_STAGE_ACC, STAT_STAGE_MAX, 2);
			break;

		case EFFECT_EVASION_UP:
		case EFFECT_EVASION_UP_2:
		case EFFECT_MINIMIZE:
			if (IsMovePredictionPhazingMove(bankDef, bankAtk))
				break;
			if (move != MOVE_ACUPRESSURE)
			{
				if (atkAbility != ABILITY_CONTRARY)
					INCREASE_STAT_VIABILITY(STAT_STAGE_EVASION, STAT_STAGE_MAX, 4);
			}
			break;

		//Decreased stat effects
		case EFFECT_ATTACK_DOWN:
		case EFFECT_ATTACK_DOWN_2:
		AI_ATTACK_MINUS:
			if (GoodIdeaToLowerAttack(bankDef, bankAtk, move))
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_DEFENSE_DOWN:
		case EFFECT_DEFENSE_DOWN_2:
		AI_DEFENSE_MINUS:
			if (GoodIdeaToLowerDefense(bankDef, bankAtk, move))
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_SPEED_DOWN:
		case EFFECT_SPEED_DOWN_2:
		AI_SPEED_MINUS:
			if (GoodIdeaToLowerSpeed(bankDef, bankAtk, move))
				INCREASE_STATUS_VIABILITY(2);
			break;

		case EFFECT_SPECIAL_ATTACK_DOWN:
		case EFFECT_SPECIAL_ATTACK_DOWN_2:
		AI_SPECIAL_ATTACK_MINUS:
			if (GoodIdeaToLowerSpAtk(bankDef, bankAtk, move))
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_SPECIAL_DEFENSE_DOWN:
		case EFFECT_SPECIAL_DEFENSE_DOWN_2:
		AI_SPECIAL_DEFENSE_MINUS:
			if (GoodIdeaToLowerSpDef(bankDef, bankAtk, move))
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_ACCURACY_DOWN:
		case EFFECT_ACCURACY_DOWN_2:
		AI_ACCURACY_MINUS:
			if (GoodIdeaToLowerAccuracy(bankDef, bankAtk, move))
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_EVASION_DOWN:
		case EFFECT_EVASION_DOWN_2:
		AI_EVASION_MINUS:
			if (GoodIdeaToLowerEvasion(bankDef, bankAtk, move))
				INCREASE_STATUS_VIABILITY(2);
			break;

		case EFFECT_HAZE:
			if (ShouldPhaze(bankAtk, bankDef, move, class))
				INCREASE_VIABILITY(8);
			break;

		/*
		case EFFECT_BIDE:
			if (DamagingMoveInMoveset(bankAtk))
				break;
			else if (IsClassSweeper(class))
				viability += 3;
			break;*/

		case EFFECT_ROAR:
			if (ShouldPhaze(bankAtk, bankDef, move, class))
				INCREASE_VIABILITY(8);
			break;

		case EFFECT_MULTI_HIT:
		case EFFECT_TRIPLE_KICK:
		case EFFECT_DOUBLE_HIT: ;
			u8 strongestMoveEffect = gBattleMoves[GetStrongestMove(bankAtk, bankDef)].effect;

			if (IS_SINGLE_BATTLE
			&& IsClassSweeper(class)
			&& !IsStrongestMove(move, bankAtk, bankDef) //This multi-hit move wouldn't be chosen normally
			&& strongestMoveEffect != EFFECT_MULTI_HIT //The strongest move isn't a multi-hit move
			&& strongestMoveEffect != EFFECT_TRIPLE_KICK
			&& strongestMoveEffect != EFFECT_DOUBLE_HIT
			&& (MoveBlockedBySubstitute(move, bankAtk, bankDef) //Attack has to hit substitute to break it
			 || data->atkItemEffect == ITEM_EFFECT_FLINCH))
				INCREASE_VIABILITY(3); //Move past strongest move
			break;

		case EFFECT_CONVERSION:
			if (!IsOfType(bankAtk, gBattleMoves[gBattleMons[bankAtk].moves[0]].type)
			&& IsClassSweeper(class))
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_FLINCH_HIT:
		AI_FLINCH_CHECKS:
			if (defAbility == ABILITY_INNERFOCUS
			||  MoveBlockedBySubstitute(move, bankAtk, bankDef)
			||  !MoveWouldHitFirst(move, bankAtk, bankDef))
				break;
			else if (atkAbility == ABILITY_SERENEGRACE
			|| data->defStatus1 & STATUS1_PARALYSIS
			|| data->defStatus2 & STATUS2_INFATUATION
			|| IsConfused(bankDef))
			{
				if (IsUsefulToFlinchTarget(bankDef))
					INCREASE_STATUS_VIABILITY(3);
			}
			break;

		case EFFECT_RESTORE_HP:
		case EFFECT_MORNING_SUN:
		case EFFECT_SWALLOW:
		AI_RECOVER:
			if (ShouldRecover(bankAtk, bankDef, move))
			{
				if (IsClassStall(class))
					INCREASE_VIABILITY(4);
				else if (IsClassDoublesTrickRoomSetup(class))
					INCREASE_VIABILITY(16);
				else
					INCREASE_STATUS_VIABILITY(3);
			}
			break;

		case EFFECT_TOXIC:
		case EFFECT_POISON:
		AI_POISON_CHECKS:
			if (!BadIdeaToPoison(bankDef, bankAtk))
			{
				if (MoveInMoveset(MOVE_VENOSHOCK, bankAtk)
				||  MoveInMoveset(MOVE_HEX, bankAtk)
				||  MoveInMoveset(MOVE_VENOMDRENCH, bankAtk)
				||  atkAbility == ABILITY_MERCILESS)
					INCREASE_STATUS_VIABILITY(2);
				else
					INCREASE_STATUS_VIABILITY(1); //AI enjoys poisoning
			}
			break;

		case EFFECT_LIGHT_SCREEN:
			if (ShouldSetUpScreens(bankAtk, bankDef, move))
				INCREASE_VIABILITY(7);
			break;

		case EFFECT_REST:
			if (!(CanBePutToSleep(bankAtk, FALSE)))
				break;
			else if (data->atkItemEffect == ITEM_EFFECT_CURE_SLP
			|| data->atkItemEffect == ITEM_EFFECT_CURE_STATUS
			|| MoveEffectInMoveset(EFFECT_SLEEP_TALK, bankAtk)
			|| MoveEffectInMoveset(EFFECT_SNORE, bankAtk)
			|| atkAbility == ABILITY_SHEDSKIN
			|| atkAbility == ABILITY_EARLYBIRD
			|| (gBattleWeather & WEATHER_RAIN_ANY && gWishFutureKnock.weatherDuration != 1 && atkAbility == ABILITY_HYDRATION && data->atkItemEffect != ITEM_EFFECT_UTILITY_UMBRELLA))
			{
				if (ShouldRecover(bankAtk, bankDef, move))
					INCREASE_STATUS_VIABILITY(1);
			}
			else
				goto AI_RECOVER;

			break;

		case EFFECT_TRAP:
		AI_TRAP:
			if (MoveInMoveset(MOVE_RAPIDSPIN, bankDef)
			||  IsOfType(bankDef, TYPE_GHOST)
			||  data->defStatus2 & (STATUS2_WRAPPED))
			{
				break; //Just a regular attacking move now
			}
			else if (ShouldTrap(bankAtk, bankDef, move, class))
				INCREASE_VIABILITY(8);
			break;

		case EFFECT_MIST:
			if (IS_SINGLE_BATTLE)
			{
				if (IsClassScreener(class))
					INCREASE_VIABILITY(6);
			}
			else //Double Battle
			{
				if (IsClassDoublesTeamSupport(class) && ViableMonCountFromBank(bankDef) > 1) //Just try to kill last foe
					INCREASE_VIABILITY(8);
			}
			break;

		case EFFECT_FOCUS_ENERGY:
		AI_FOCUS_ENERGY:
			if (atkAbility == ABILITY_SUPERLUCK
			|| atkAbility == ABILITY_SNIPER
			|| data->atkItemEffect == ITEM_EFFECT_SCOPE_LENS)
				INCREASE_STATUS_VIABILITY(2);
			else
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_CONFUSE:
		AI_CONFUSE_CHECK:
			if (CanBeConfused(bankDef, TRUE)
			&&  data->defItemEffect != ITEM_EFFECT_CURE_CONFUSION
			&&  data->defItemEffect != ITEM_EFFECT_CURE_STATUS)
			{
				if (data->defStatus1 & STATUS1_PARALYSIS
				|| data->defStatus2 & (STATUS2_INFATUATION)
				|| (atkAbility == ABILITY_SERENEGRACE && FlinchingMoveInMoveset(bankAtk)))
					INCREASE_STATUS_VIABILITY(2);
				else
					INCREASE_STATUS_VIABILITY(1);
			}
			/* Automatically runs the above check for target's partner too
			else if (IS_DOUBLE_BATTLE
			&& gBattleMoves[move].target & MOVE_TARGET_BOTH && CanBeConfused(data->bankDefPartner))
			{
				if (defPartnerStatus1 & STATUS1_PARALYSIS
				|| defPartnerStatus2 & (STATUS2_INFATUATION)
				|| (atkAbility == ABILITY_SERENEGRACE && FlinchingMoveInMoveset(bankAtk)))
					INCREASE_STATUS_VIABILITY(2);
				else
					INCREASE_STATUS_VIABILITY(1);
			}*/
			break;

		case EFFECT_REFLECT:
			switch (move) {
				case MOVE_AURORAVEIL:
				case MOVE_G_MAX_RESONANCE_P:
				case MOVE_G_MAX_RESONANCE_S:
					if (ShouldSetUpScreens(bankAtk, bankDef, move))
					{
						if (IsClassScreener(class))
							INCREASE_VIABILITY(8);
						else
							INCREASE_STATUS_VIABILITY(2);
					}
					break;

				default:
					if (ShouldSetUpScreens(bankAtk, bankDef, move))
					{
						if (IsClassScreener(class))
							INCREASE_VIABILITY(7);
						else
							INCREASE_STATUS_VIABILITY(2);
					}
			}
			break;

		case EFFECT_PARALYZE:
		AI_PARALYZE_CHECKS:
			if (!BadIdeaToParalyze(bankDef, bankAtk))
			{
				u8 atkSpeedCalc = SpeedCalc(bankAtk);
				u8 defSpeedCalc = SpeedCalc(bankDef);

				if (IncreaseViabilityForSpeedControl(&viability, class, bankAtk, bankDef))
					break;

				else if ((defSpeedCalc >= atkSpeedCalc && defSpeedCalc / 2 < atkSpeedCalc) //You'll go first after paralyzing foe
				|| MoveInMoveset(MOVE_HEX, bankAtk)
				|| FlinchingMoveInMoveset(bankAtk)
				|| data->defStatus2 & STATUS2_INFATUATION
				|| IsConfused(bankDef))
					INCREASE_STATUS_VIABILITY(2);
				else
					INCREASE_STATUS_VIABILITY(1);
			}
			break;

		//For the stat down hit, only rely on these gimmicks in Single battles
		case EFFECT_ATTACK_DOWN_HIT:
			if (IS_SINGLE_BATTLE && CalcSecondaryEffectChance(bankAtk, move) >= 50)
				goto AI_ATTACK_MINUS;
			break;

		case EFFECT_DEFENSE_DOWN_HIT:
			if (IS_SINGLE_BATTLE && CalcSecondaryEffectChance(bankAtk, move) >= 50)
				goto AI_DEFENSE_MINUS;
			break;

		case EFFECT_SPEED_DOWN_HIT:
			if (CalcSecondaryEffectChance(bankAtk, move) >= 50)
			{
				if (IS_SINGLE_BATTLE)
					goto AI_SPEED_MINUS;
				else //Double Battle
				{
					if (defAbility != ABILITY_CONTRARY
					&& defAbility != ABILITY_CLEARBODY
					&& defAbility != ABILITY_WHITESMOKE
					//&& defAbility != ABILITY_FULLMETALBODY
					&& STAT_STAGE(bankDef, STAT_STAGE_SPEED) > 0)
						IncreaseViabilityForSpeedControl(&viability, class, bankAtk, bankDef);
				}
			}
			break;

		case EFFECT_SPECIAL_ATTACK_DOWN_HIT:
			if (IS_SINGLE_BATTLE && CalcSecondaryEffectChance(bankAtk, move) >= 50)
				goto AI_SPECIAL_ATTACK_MINUS;
			break;

		case EFFECT_SPECIAL_DEFENSE_DOWN_HIT:
			if (IS_SINGLE_BATTLE && CalcSecondaryEffectChance(bankAtk, move) >= 50)
				goto AI_SPECIAL_DEFENSE_MINUS;
			break;

		case EFFECT_ACCURACY_DOWN_HIT:
			if (IS_SINGLE_BATTLE && CalcSecondaryEffectChance(bankAtk, move) >= 50)
				goto AI_ACCURACY_MINUS;
			break;

		case EFFECT_EVASION_DOWN_HIT:
			if (IS_SINGLE_BATTLE && CalcSecondaryEffectChance(bankAtk, move) >= 50)
				goto AI_EVASION_MINUS;
			break;

		case EFFECT_CONFUSE_HIT:
			if (CalcSecondaryEffectChance(bankAtk, move) >= 75
			&&  MoveWillHit(move, bankAtk, bankDef))
				goto AI_CONFUSE_CHECK;
			break;

		case EFFECT_SUBSTITUTE:
			IncreaseSubstituteViability(&viability, class, bankAtk, bankDef);
			break;

		case EFFECT_MIMIC:
			if (MoveWouldHitFirst(move, bankAtk, bankDef))
			{
				if (gLastUsedMoves[bankDef] != MOVE_NONE
				&& gLastUsedMoves[bankDef] != 0xFFFF)
				{
					if (!CanKnockOut(bankDef, bankAtk)
					&& MoveKnocksOutXHits(gLastUsedMoves[bankDef], bankAtk, bankDef, 1))
						INCREASE_STATUS_VIABILITY(2);
					else if (MoveEffectInMoveset(EFFECT_IMPRISON, bankAtk))
						INCREASE_STATUS_VIABILITY(1);
				}

			}
			else if (predictedMove != MOVE_NONE)
			{
				if (!Can2HKO(bankDef, bankAtk)
				&& MoveKnocksOutXHits(predictedMove, bankAtk, bankDef, 1)) //Attacker can kill with mimicked move
					INCREASE_STATUS_VIABILITY(1);
				else if (MoveEffectInMoveset(EFFECT_IMPRISON, bankAtk))
					INCREASE_STATUS_VIABILITY(1);
			}
			break;

		case EFFECT_LEECH_SEED:
			if (IsOfType(bankDef, TYPE_GRASS)
			|| data->defStatus3 & STATUS3_LEECHSEED
			|| MoveInMoveset(MOVE_RAPIDSPIN, bankDef)
			|| defAbility == ABILITY_LIQUIDOOZE
			|| defAbility == ABILITY_MAGICGUARD)
				break;
			else
				INCREASE_STATUS_VIABILITY(3);
			break;

		case EFFECT_SPLASH:
			if (IsTypeZCrystal(data->atkItem, moveType) && !IsMegaZMoveBannedBattle() && !gNewBS->zMoveData.used[bankAtk])
				INCREASE_VIABILITY(9); //Z-Splash!
			break;
		
		case EFFECT_TELEPORT:
			if (gBattleTypeFlags & BATTLE_TYPE_TRAINER || SIDE(gBankAttacker) == B_SIDE_PLAYER)
				goto PIVOT_CHECK;
			break;

		case EFFECT_DISABLE:
			if (gDisableStructs[bankDef].disableTimer1 == 0
			&&  data->defItemEffect != ITEM_EFFECT_CURE_ATTRACT)
			{
				if (MoveWouldHitFirst(move, bankAtk, bankDef))
				{
					if (gLastUsedMoves[bankDef] != MOVE_NONE
					&& gLastUsedMoves[bankDef] != 0xFFFF)
					{
						if (gLastUsedMoves[bankDef] == predictedMove)
							INCREASE_STATUS_VIABILITY(3);
						else if (MoveKnocksOutXHits(gLastUsedMoves[bankDef], bankDef, bankAtk, 1))
							INCREASE_STATUS_VIABILITY(2); //Disable move that can kill attacker
					}
				}
				else if (predictedMove != MOVE_NONE && SPLIT(predictedMove) == SPLIT_STATUS)
					INCREASE_STATUS_VIABILITY(1); //Disable annoying status moves
			}
			break;

		case EFFECT_ENCORE:
			if (gDisableStructs[bankDef].encoreTimer == 0
			&&  data->defItemEffect != ITEM_EFFECT_CURE_ATTRACT)
			{
				if (MoveWouldHitFirst(move, bankAtk, bankDef))
				{
					if (gLastUsedMoves[bankDef] != MOVE_NONE
					&& gLastUsedMoves[bankDef] != 0xFFFF
					&& (SPLIT(gLastUsedMoves[bankDef]) == SPLIT_STATUS || AI_SpecialTypeCalc(gLastUsedMoves[bankDef], bankDef, bankAtk) & MOVE_RESULT_DOESNT_AFFECT_FOE))
						INCREASE_STATUS_VIABILITY(3); //Lock into status moves
				}
				else if (predictedMove != MOVE_NONE && SPLIT(predictedMove) == SPLIT_STATUS)
					INCREASE_STATUS_VIABILITY(3); //Lock into status moves
			}
			break;

		case EFFECT_PAIN_SPLIT: ;
			u16 newHealth = (gBattleMons[bankAtk].hp + gBattleMons[bankDef].hp) / 2;
			u16 healthBenchmark = (gBattleMons[bankAtk].hp * 12) / 10;

			if (newHealth > healthBenchmark)
			{
				goto AI_DRAIN_HP_CHECK;
			}

			break;

		case EFFECT_SNORE:
		case EFFECT_SLEEP_TALK:
			if (data->atkStatus1 & STATUS1_SLEEP)
				INCREASE_VIABILITY(10);
			break;

		case EFFECT_LOCK_ON:
			switch (move) {
				case MOVE_LASERFOCUS:
					goto AI_FOCUS_ENERGY;

				default:
					if (MoveEffectInMoveset(EFFECT_0HKO, bankAtk))
						INCREASE_STATUS_VIABILITY(2);
			}
			break;

		case EFFECT_SPEED_UP_1_HIT:
			if (CalcSecondaryEffectChance(bankAtk, move) >= 75)
				goto AI_SPEED_PLUS;
			break;

		case EFFECT_DESTINY_BOND:
			if (MoveWouldHitFirst(move, bankAtk, bankDef) && CanKnockOut(bankDef, bankAtk))
				INCREASE_STATUS_VIABILITY(3);
			break;

		case EFFECT_SPITE: ;
			u8 moveSlot = FindMovePositionInMoveset(predictedMove, bankDef);

			if (MoveWouldHitFirst(move, bankAtk, bankDef)
			&&  predictedMove != MOVE_NONE
			&&  moveSlot < 4
			&&  gBattleMons[bankDef].pp[moveSlot] <= 4)
				INCREASE_STATUS_VIABILITY(3);
			break;

		case EFFECT_HEAL_BELL:
			if (ShouldUseWishAromatherapy(bankAtk, bankDef, move, class))
				INCREASE_VIABILITY(7);
			break;

		case EFFECT_THIEF:
			if (data->atkItem == ITEM_NONE
			&&  data->defItem != ITEM_NONE
			&& CanTransferItem(SPECIES(bankDef), data->defItem)
			&& CanTransferItem(SPECIES(bankAtk), data->defItem)
			&& !MoveInMoveset(MOVE_ACROBATICS, bankAtk)
			&& defAbility != ABILITY_STICKYHOLD
			&& IsClassSweeper(class))
			{
				switch (ItemId_GetHoldEffect(ITEM(bankDef))) {
					case ITEM_EFFECT_CHOICE_BAND: ;
						if (ITEM_QUALITY(bankDef) == QUALITY_CHOICE_SCARF
						|| MoveSplitInMoveset(ITEM_QUALITY(bankDef), bankAtk))
							INCREASE_STATUS_VIABILITY(2);
						break;

					case ITEM_EFFECT_TOXIC_ORB:
						if (GoodIdeaToPoisonSelf(bankAtk))
							INCREASE_STATUS_VIABILITY(2);
						break;

					case ITEM_EFFECT_FLAME_ORB:
						if (GoodIdeaToBurnSelf(bankAtk))
							INCREASE_STATUS_VIABILITY(2);
						break;

					case ITEM_EFFECT_BLACK_SLUDGE:
						if (IsOfType(bankAtk, TYPE_POISON))
							INCREASE_STATUS_VIABILITY(2);
						break;

					case ITEM_EFFECT_IRON_BALL:
						if (MoveInMoveset(MOVE_FLING, bankAtk))
							INCREASE_STATUS_VIABILITY(2);
						break;

					case ITEM_EFFECT_LAGGING_TAIL:
					case ITEM_EFFECT_STICKY_BARB:
						break;

					default:
						INCREASE_VIABILITY(1);
				}
			}
			break;

		case EFFECT_MEAN_LOOK:
			if (IsTrapped(bankDef, TRUE))
				break; //Already trapped

			goto AI_TRAP;

		case EFFECT_NIGHTMARE:
			if (defAbility == ABILITY_MAGICGUARD)
				break;
			if (defAbility == ABILITY_COMATOSE || data->defStatus1 & STATUS1_SLEEP)
				INCREASE_STATUS_VIABILITY(3);
			break;

		case EFFECT_CURSE:
			if (IsOfType(bankAtk, TYPE_GHOST))
			{
				if (IsTrapped(bankDef, TRUE))
					INCREASE_STATUS_VIABILITY(3);
				else
					INCREASE_STATUS_VIABILITY(1);
				break;
			}
			else
			{
				if (atkAbility == ABILITY_CONTRARY || defAbility == ABILITY_MAGICGUARD)
					break;
				else if (MoveInMoveset(MOVE_GYROBALL, bankAtk))
					INCREASE_STAT_VIABILITY(STAT_STAGE_ATK, STAT_STAGE_MAX, 4); //Go until Gyro Ball can kill in 1 hit
				else if (STAT_STAGE(bankAtk, STAT_STAGE_SPEED) < 3)
					break;
				else if (STAT_STAGE(bankAtk, STAT_STAGE_ATK) < 8)
					INCREASE_STAT_VIABILITY(STAT_STAGE_ATK, 8, 2);
				else if (STAT_STAGE(bankAtk, STAT_STAGE_DEF) < 8)
					INCREASE_STAT_VIABILITY(STAT_STAGE_DEF, 8, 1);
				break;
			}
			break;

		case EFFECT_PROTECT: ;
			switch (move)
			{
				case MOVE_QUICKGUARD:
					if (predictedMove != MOVE_NONE && PriorityCalc(bankDef, ACTION_USE_MOVE, predictedMove) > 0)
					{
						if (IS_SINGLE_BATTLE)
						{
							move = MOVE_PROTECT; //Treat move like Protect
							goto PROTECT_CHECKS;
						}
						else
							IncreaseTeamProtectionViability(&viability, class);
					}
					break;

				case MOVE_WIDEGUARD:
					if (predictedMove != MOVE_NONE && gBattleMoves[predictedMove].target & (MOVE_TARGET_ALL | MOVE_TARGET_BOTH))
					{
						if (IS_SINGLE_BATTLE)
						{
							move = MOVE_PROTECT; //Treat move like Protect
							goto PROTECT_CHECKS;
						}
						else
							IncreaseTeamProtectionViability(&viability, class);
					}
					else if (IS_DOUBLE_BATTLE && gBattleMoves[data->partnerMove].target & MOVE_TARGET_ALL)
					{
						if (atkAbility != ABILITY_TELEPATHY
						&& !(AI_SpecialTypeCalc(data->partnerMove, data->bankAtkPartner, bankAtk) & MOVE_RESULT_NO_EFFECT)) //Move has effect
							IncreaseAllyProtectionViability(&viability, class); //Protect against partner move
					}
					break;

				case MOVE_CRAFTYSHIELD:
					if (predictedMove != MOVE_NONE && SPLIT(predictedMove) == SPLIT_STATUS && !(gBattleMoves[predictedMove].target & MOVE_TARGET_USER))
					{
						if (IS_SINGLE_BATTLE)
						{
							move = MOVE_PROTECT; //Treat move like Protect
							goto PROTECT_CHECKS;
						}
						else
							IncreaseTeamProtectionViability(&viability, class);
					}
					break;

				case MOVE_MATBLOCK:
					if (gDisableStructs[bankAtk].isFirstTurn && predictedMove != MOVE_NONE && SPLIT(predictedMove) != SPLIT_STATUS && !(gBattleMoves[predictedMove].target & MOVE_TARGET_USER))
					{
						if (IS_SINGLE_BATTLE)
						{
							move = MOVE_PROTECT; //Treat move like Protect
							goto PROTECT_CHECKS;
						}
						else
							IncreaseTeamProtectionViability(&viability, class);
					}
					break;

				case MOVE_ENDURE:
					if (CanKnockOut(bankDef, bankAtk))
					{
						if (gBattleMons[bankAtk].hp > gBattleMons[bankAtk].maxHP / 4 //Pinch berry couldn't have activated yet
						&& IsPinchBerryItemEffect(data->atkItemEffect))
						{
							INCREASE_STATUS_VIABILITY(3);
						}
						else if (gBattleMons[bankAtk].hp > 1 //Only spam endure for Flail/Reversal if you're not at Min Health
						&& CanKnockOut(bankDef, bankAtk))
						{
							if (MoveEffectInMoveset(EFFECT_FLAIL, bankAtk)
							||  MoveEffectInMoveset(EFFECT_ENDEAVOR, bankAtk))
								INCREASE_STATUS_VIABILITY(3);
						}
					}
					break;

				case MOVE_KINGSSHIELD:
					#if (defined SPECIES_AEGISLASH && defined SPECIES_AEGISLASH_BLADE)
					if (atkAbility == ABILITY_STANCECHANGE //Special logic for Aegislash
					&&  data->atkSpecies == SPECIES_AEGISLASH_BLADE
					&&  !IsBankIncapacitated(bankDef))
					{
						if (IsClassStall(class))
							INCREASE_VIABILITY(3);
						else if (IS_DOUBLE_BATTLE)
							INCREASE_VIABILITY(19);
						else
							INCREASE_STATUS_VIABILITY(3);
						break;
					}
					#endif
					__attribute__ ((fallthrough));

				default:
				PROTECT_CHECKS: ;
					u8 shouldProtect = ShouldProtect(bankAtk, bankDef, move);

					if (shouldProtect == USE_PROTECT || shouldProtect == PROTECT_FROM_FOES)
					{
						IncreaseFoeProtectionViability(&viability, class, bankAtk, bankDef);
					}
					else if (shouldProtect == PROTECT_FROM_ALLIES)
					{
						IncreaseAllyProtectionViability(&viability, class);
					}
			}
			break;

		case EFFECT_SPIKES: ;
			u8 firstId, lastId;
			struct Pokemon* atkParty = LoadPartyRange(bankAtk, &firstId, &lastId);
			struct Pokemon* defParty = LoadPartyRange(bankDef, &firstId, &lastId);

			switch (move) {
				case MOVE_STICKYWEB:
					for (i = 0; i < PARTY_SIZE; ++i) //Loop through attacker party
					{
						if (GetMonData(&atkParty[i], MON_DATA_HP, NULL) > 0
						&&  GetMonData(&atkParty[i], MON_DATA_SPECIES, NULL) != SPECIES_NONE
						&& !GetMonData(&atkParty[i], MON_DATA_IS_EGG, NULL))
						{
							for (j = 0; j < PARTY_SIZE; ++j) //Loop through target party
							{
								if (GetMonData(&defParty[j], MON_DATA_HP, NULL) > 0
								&&  GetMonData(&defParty[j], MON_DATA_SPECIES, NULL) != SPECIES_NONE
								&& !GetMonData(&defParty[j], MON_DATA_IS_EGG, NULL)
								&&  ItemId_GetHoldEffect(GetMonData(&defParty[j], MON_DATA_HELD_ITEM, NULL)) != ITEM_EFFECT_HEAVY_DUTY_BOOTS
								&&  j != gBattlerPartyIndexes[data->foe1]
								&&  j != gBattlerPartyIndexes[data->foe2]
								&&  CheckGroundingFromPartyData(&defParty[j]) == GROUNDED //Affected by Sticky Web
								&&  SpeedCalcMon(SIDE(bankAtk), &atkParty[i]) < SpeedCalcMon(SIDE(bankDef), &defParty[j]))
								{
									IncreaseEntryHazardsViability(&viability, class, bankAtk, bankDef, move);
									goto END_ENTRY_HAZARDS;
								}
							}
						}
					}
					break;

				case MOVE_STEALTHROCK:
				case MOVE_G_MAX_STONESURGE_P:
				case MOVE_G_MAX_STONESURGE_S:
				case MOVE_G_MAX_STEELSURGE_P:
				case MOVE_G_MAX_STEELSURGE_S:
					for (i = 0; i < PARTY_SIZE; ++i)
					{
						if (GetMonData(&defParty[i], MON_DATA_SPECIES, NULL) != SPECIES_NONE
						&& !GetMonData(&defParty[i], MON_DATA_IS_EGG, NULL)
						&&  GetMonData(&defParty[i], MON_DATA_HP, NULL) > 0
						&&  ItemId_GetHoldEffect(GetMonData(&defParty[i], MON_DATA_HELD_ITEM, NULL)) != ITEM_EFFECT_HEAVY_DUTY_BOOTS
						&&  i != gBattlerPartyIndexes[data->foe1]
						&&  i != gBattlerPartyIndexes[data->foe2])
						{
							IncreaseEntryHazardsViability(&viability, class, bankAtk, bankDef, move);
							break; //Can hurt at least one mon
						}
					}
					break;

				case MOVE_TOXICSPIKES:
					for (i = 0; i < PARTY_SIZE; ++i)
					{
						if (GetMonData(&defParty[i], MON_DATA_SPECIES, NULL) != SPECIES_NONE
						&& !GetMonData(&defParty[i], MON_DATA_IS_EGG, NULL)
						&&  GetMonData(&defParty[i], MON_DATA_HP, NULL) > 0
						&&  ItemId_GetHoldEffect(GetMonData(&defParty[i], MON_DATA_HELD_ITEM, NULL)) != ITEM_EFFECT_HEAVY_DUTY_BOOTS
						&&  i != gBattlerPartyIndexes[data->foe1]
						&&  i != gBattlerPartyIndexes[data->foe2]
						&&  CheckGroundingFromPartyData(&defParty[i]) == GROUNDED)
						{
							u8 type1 = (gBattleTypeFlags & BATTLE_TYPE_CAMOMONS) ? GetCamomonsTypeByMon(&defParty[i], 0) : gBaseStats[defParty[i].species].type1;
							u8 type2 = (gBattleTypeFlags & BATTLE_TYPE_CAMOMONS) ? GetCamomonsTypeByMon(&defParty[i], 1) : gBaseStats[defParty[i].species].type2;
							if (type1 == TYPE_POISON || type2 == TYPE_POISON)
								break; //Don't set up Toxic spikes if someone in party who can just remove them

							if (CanPartyMonBePoisoned(&defParty[i]))
							{
								IncreaseEntryHazardsViability(&viability, class, bankAtk, bankDef, move);
								break;
							}
						}
					}
					break;

				default: //Spikes
					for (i = 0; i < PARTY_SIZE; ++i)
					{
						if (GetMonData(&defParty[i], MON_DATA_SPECIES, NULL) != SPECIES_NONE
						&& !GetMonData(&defParty[i], MON_DATA_IS_EGG, NULL)
						&&  GetMonData(&defParty[i], MON_DATA_HP, NULL) > 0
						&&  ItemId_GetHoldEffect(GetMonData(&defParty[i], MON_DATA_HELD_ITEM, NULL)) != ITEM_EFFECT_HEAVY_DUTY_BOOTS
						&&  i != gBattlerPartyIndexes[data->foe1]
						&&  i != gBattlerPartyIndexes[data->foe2]
						&&  CheckGroundingFromPartyData(&defParty[i]) == GROUNDED)
						{
							IncreaseEntryHazardsViability(&viability, class, bankAtk, bankDef, move);
							break; //Can hurt at least one mon
						}
					}
					break;
			}

		END_ENTRY_HAZARDS:
			break;

		case EFFECT_FORESIGHT:
			if (move == MOVE_MIRACLEEYE)
			{
				if (STAT_STAGE(bankDef, STAT_STAGE_EVASION) > 6
				|| (IsOfType(bankDef, TYPE_DARK) && (MoveTypeInMoveset(TYPE_PSYCHIC, bankAtk))))
					INCREASE_STATUS_VIABILITY(2);
				break;
			}
			else
			{
				if (atkAbility == ABILITY_SCRAPPY)
					break;
				else if (STAT_STAGE(bankDef, STAT_STAGE_EVASION) > 6
				|| (IsOfType(bankDef, TYPE_GHOST)
				 && (MoveTypeInMoveset(TYPE_NORMAL, bankAtk)
				  || MoveTypeInMoveset(TYPE_FIGHTING, bankAtk))))
					INCREASE_STATUS_VIABILITY(2);
				break;
			}
			break;

		case EFFECT_PERISH_SONG:
			if (IsTrapped(bankDef, TRUE))
				INCREASE_STATUS_VIABILITY(3);
			break;

		case EFFECT_SANDSTORM:
			if (IsClassDoublesSetupAttacker(class)
			&& IsTypeZCrystal(data->atkItem, moveType)
			&& !gNewBS->zMoveData.used[bankAtk] //Z-Crystal provides speed up
			&& !IsMegaZMoveBannedBattle()
			&& atkAbility != ABILITY_CONTRARY)
				IncreaseTailwindViability(&viability, class, bankAtk, bankDef);

			else if (IsClassDoublesSetupAttacker(class)
			&& atkAbility == ABILITY_SANDRUSH)
				IncreaseTailwindViability(&viability, class, bankAtk, bankDef);

			else if (atkAbility == ABILITY_SANDVEIL
			|| atkAbility == ABILITY_SANDRUSH
			|| atkAbility == ABILITY_SANDFORCE
			|| atkAbility == ABILITY_SANDFORCE
			|| atkAbility == ABILITY_OVERCOAT
			|| atkAbility == ABILITY_MAGICGUARD
			|| data->atkItemEffect == ITEM_EFFECT_SAFETY_GOGGLES
			|| IsOfType(bankAtk, TYPE_ROCK)
			|| IsOfType(bankAtk, TYPE_STEEL)
			|| IsOfType(bankAtk, TYPE_GROUND)
			|| MoveInMoveset(MOVE_SHOREUP, bankAtk)
			|| MoveInMoveset(MOVE_WEATHERBALL, bankAtk)
			|| data->atkItemEffect == ITEM_EFFECT_SMOOTH_ROCK)
			{
				if (IsClassDoublesTeamSupport(class))
					INCREASE_VIABILITY(17);
				else
					INCREASE_STATUS_VIABILITY(2);
			}
			break;

		case EFFECT_ROLLOUT:
			if (IsClassSweeper(class)
			&& data->atkStatus2 & STATUS2_DEFENSE_CURL)
				INCREASE_VIABILITY(8);
			break;

		case EFFECT_SWAGGER:
			if (MoveInMoveset(MOVE_FOULPLAY, bankAtk)
			||  MoveEffectInMoveset(EFFECT_PSYCH_UP, bankAtk)) //Includes Spectral Thief
				INCREASE_STATUS_VIABILITY(2);
			else
				goto AI_CONFUSE_CHECK;
			break;

		case EFFECT_FLATTER:
			if (MoveEffectInMoveset(EFFECT_PSYCH_UP, bankAtk)) //Includes Spectral Thief
				INCREASE_STATUS_VIABILITY(2);
			else
				goto AI_CONFUSE_CHECK;
			break;

		case EFFECT_FURY_CUTTER:
			if (IS_SINGLE_BATTLE
			&& IsClassSweeper(class)
			&& data->atkItemEffect == ITEM_EFFECT_METRONOME)
				INCREASE_VIABILITY(3); //Past strongest move
			break;

		case EFFECT_ATTRACT:
			if (IS_SINGLE_BATTLE
			&&  WillFaintFromSecondaryDamage(bankDef)
			&& !MoveWouldHitFirst(move, bankAtk, bankDef))
				break; //Don't use if the attract will never get a chance to proc

			if (data->defStatus1 & STATUS1_ANY
			|| IsConfused(bankDef)
			|| IsTrapped(bankDef, TRUE))
				INCREASE_STATUS_VIABILITY(2);
			else
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_SAFEGUARD:
			if (IS_SINGLE_BATTLE)
			{
				if (IsClassTeamSupport(class)
				&& (gTerrainType != MISTY_TERRAIN || !CheckGrounding(bankAtk)))
					INCREASE_STATUS_VIABILITY(1);
			}
			else //Double Battle
			{
				if (IsClassDoublesTeamSupport(class) && ViableMonCountFromBank(bankDef) > 1) //Just try to kill last foe
					INCREASE_VIABILITY(8);
			}
			break;

		case EFFECT_BATON_PASS:
			switch (move)
			{
				case MOVE_UTURN:
				case MOVE_VOLTSWITCH:
				case MOVE_FLIPTURN:
				case MOVE_PARTINGSHOT:
					PIVOT_CHECK:
					if (IS_SINGLE_BATTLE)
					{
						u8 shouldPivot = ShouldPivot(bankAtk, bankDef, move, class);
						if (shouldPivot == PIVOT)
							IncreasePivotViability(&viability, class, bankAtk, bankDef);
						else if (shouldPivot == DONT_PIVOT)
							DECREASE_VIABILITY(10); //Bad idea to use this move
						else if (gWishFutureKnock.wishCounter[bankAtk] > 0
							  && ShouldUseWishAromatherapy(bankAtk, bankDef, MOVE_WISH, class))
						{
							INCREASE_VIABILITY(7);
						}
					}
					else //Double Battle
					{
						if (!BankHasMonToSwitchTo(bankAtk))
							break; //Can't switch

						if (GetMonAbility(GetBankPartyData(bankAtk)) == ABILITY_INTIMIDATE
						&&  MoveSplitOnTeam(bankDef, SPLIT_PHYSICAL))
						{
							if (IsClassDoublesUtility(class))
								INCREASE_VIABILITY(16);
							else if (IsClassDoublesAllOutAttacker(class))
								INCREASE_VIABILITY(18);
						}
					}
					break;

				case MOVE_BATONPASS:
					if (IsClassBatonPass(class))
						INCREASE_VIABILITY(3);
					break;
			}
			break;

		case EFFECT_PURSUIT:
			if (IsClassSweeper(class))
			{
				if (IsPredictedToSwitch(bankDef, bankAtk))
					INCREASE_VIABILITY(3);
				else if (IsPredictedToUsePursuitableMove(bankDef, bankAtk) && !MoveWouldHitFirst(move, bankAtk, bankDef)) //Pursuit against fast U-Turn
					INCREASE_VIABILITY(3);
			}
			break;

		case EFFECT_RAPID_SPIN:
			if (gSideStatuses[SIDE(bankAtk)] & SIDE_STATUS_SPIKES)
			{
				if ((IS_SINGLE_BATTLE && ViableMonCountFromBank(bankAtk) >= 2) //Pokemon to switch out to in singles
				||  (IS_DOUBLE_BATTLE && ViableMonCountFromBank(bankAtk) >= 3)) //Pokemon to switch out to in doubles
				{
					if (IS_DOUBLE_BATTLE)
						IncreaseHelpingHandViability(&viability, class);
					else
						INCREASE_STATUS_VIABILITY(3);
					break;
				}
			}

			//At this point no entry hazards are to be removed
			if (move != MOVE_RAPIDSPIN)
			{
				if (gSideStatuses[SIDE(bankDef)] &
					(SIDE_STATUS_REFLECT | SIDE_STATUS_LIGHTSCREEN | SIDE_STATUS_SAFEGUARD | SIDE_STATUS_MIST)
				|| gNewBS->AuroraVeilTimers[SIDE(bankDef)] != 0)
				{
					if (IS_DOUBLE_BATTLE)
						IncreaseHelpingHandViability(&viability, class);
					else
						INCREASE_STATUS_VIABILITY(3);
				}
				else if (!(gSideStatuses[SIDE(bankDef)] & SIDE_STATUS_SPIKES)) //Don't blow away hazards if you set them up
				{
					if (IS_DOUBLE_BATTLE)
					{
						if (gBattleMoves[data->partnerMove].effect == EFFECT_SPIKES //Partner is going to set up hazards
						&& !MoveWouldHitBeforeOtherMove(move, bankAtk, data->partnerMove, data->bankAtkPartner)) //Partner is going to set up before the potential Defog
							break; //Don't use Defog if partner is going to set up hazards
					}

					goto AI_EVASION_MINUS; //If no other reason to use Defog, use it for the Evasion lowering
				}
			}
			else //Rapid Spin
			{
				if (data->atkStatus3 & STATUS3_LEECHSEED
				|| data->atkStatus2 & STATUS2_WRAPPED)
					INCREASE_STATUS_VIABILITY(3);
			}
			break;

		case EFFECT_RAIN_DANCE:
			if (IsClassDoublesSetupAttacker(class)
			&& IsTypeZCrystal(data->atkItem, moveType)
			&& !gNewBS->zMoveData.used[bankAtk] //Z-Crystal provides speed up
			&& !IsMegaZMoveBannedBattle()
			&& atkAbility != ABILITY_CONTRARY)
				IncreaseTailwindViability(&viability, class, bankAtk, bankDef);

			else if (IsClassDoublesSetupAttacker(class)
			&& atkAbility == ABILITY_SWIFTSWIM)
				IncreaseTailwindViability(&viability, class, bankAtk, bankDef);

			else if (data->atkItemEffect != ITEM_EFFECT_UTILITY_UMBRELLA
			&& (atkAbility == ABILITY_SWIFTSWIM
			 || atkAbility == ABILITY_FORECAST
			 || atkAbility == ABILITY_HYDRATION
			 || atkAbility == ABILITY_RAINDISH
			 || atkAbility == ABILITY_DRYSKIN
			 || MoveEffectInMoveset(EFFECT_THUNDER, bankAtk) //Includes Hurricane
			 || MoveEffectInMoveset(EFFECT_MORNING_SUN, bankDef)
			 || MoveInMoveset(MOVE_WEATHERBALL, bankAtk)
			 || MoveTypeInMoveset(TYPE_WATER, bankAtk)
			 || MoveTypeInMoveset(TYPE_FIRE, bankDef)
			 || data->atkItemEffect == ITEM_EFFECT_DAMP_ROCK))
			{
				if (IsClassDoublesTeamSupport(class))
					INCREASE_VIABILITY(17);
				else
					INCREASE_STATUS_VIABILITY(2);
			}
			break;

		case EFFECT_SUNNY_DAY:
			if (IsClassDoublesSetupAttacker(class)
			&& IsTypeZCrystal(data->atkItem, moveType)
			&& !gNewBS->zMoveData.used[bankAtk] //Z-Crystal provides speed up
			&& !IsMegaZMoveBannedBattle()
			&& atkAbility != ABILITY_CONTRARY)
				IncreaseTailwindViability(&viability, class, bankAtk, bankDef);

			else if (IsClassDoublesSetupAttacker(class)
			&& atkAbility == ABILITY_CHLOROPHYLL)
				IncreaseTailwindViability(&viability, class, bankAtk, bankDef);

			else if (data->atkItemEffect != ITEM_EFFECT_UTILITY_UMBRELLA
			&& (atkAbility == ABILITY_CHLOROPHYLL
			 || atkAbility == ABILITY_FLOWERGIFT
			 || atkAbility == ABILITY_FORECAST
			 || atkAbility == ABILITY_LEAFGUARD
			 || atkAbility == ABILITY_SOLARPOWER
			 || atkAbility == ABILITY_HARVEST
			 || MoveEffectInMoveset(EFFECT_SOLARBEAM, bankAtk)
			 || MoveEffectInMoveset(EFFECT_MORNING_SUN, bankAtk)
			 || MoveEffectInMoveset(EFFECT_THUNDER, bankDef)
			 || MoveInMoveset(MOVE_WEATHERBALL, bankAtk)
			 || MoveInMoveset(MOVE_GROWTH, bankAtk)
			 || MoveTypeInMoveset(TYPE_FIRE, bankAtk)
			 || MoveTypeInMoveset(TYPE_WATER, bankDef)
			 || data->atkItemEffect == ITEM_EFFECT_HEAT_ROCK))
			{
				if (IsClassDoublesTeamSupport(class))
					INCREASE_VIABILITY(17);
				else
					INCREASE_STATUS_VIABILITY(2);
			}
			break;

		case EFFECT_ATTACK_UP_HIT:
			if (CalcSecondaryEffectChance(bankAtk, move) >= 75)
				goto AI_ATTACK_PLUS;
			else if (move == MOVE_FELLSTINGER && STAT_STAGE(bankAtk, STAT_STAGE_ATK) < STAT_STAGE_MAX && atkAbility != ABILITY_CONTRARY)
			{
				if (MoveKnocksOutXHits(move, bankAtk, bankDef, 1))
				{
					if (IS_SINGLE_BATTLE)
					{
						if (MoveWouldHitFirst(move, bankAtk, bankDef))
							INCREASE_VIABILITY(9);
						else
							INCREASE_VIABILITY(3); //Past strongest move
					}
					else
					{
						IncreaseDoublesDamageViabilityToScore(&viability, class, 6, bankAtk, bankDef);
					}
				}
				break;
			}
			break;

		case EFFECT_BELLY_DRUM:
			if (PhysicalMoveInMoveset(bankAtk) && atkAbility != ABILITY_CONTRARY)
			{
				if (IsTypeZCrystal(data->atkItem, moveType) && !IsMegaZMoveBannedBattle())
					INCREASE_STAT_VIABILITY(STAT_STAGE_ATK, STAT_STAGE_MAX, 5);
				else
					INCREASE_STAT_VIABILITY(STAT_STAGE_ATK, 8, 2);
			}
			break;

		case EFFECT_PSYCH_UP:
			if (move != MOVE_SPECTRALTHIEF)
			{
				//Copy positive stat changes
				for (i = STAT_STAGE_ATK; i < BATTLE_STATS_NO; ++i)
				{
					if (STAT_STAGE(bankDef, i) > STAT_STAGE(bankAtk, i))
					{
						if (i == STAT_STAGE_ATK && (PhysicalMoveInMoveset(bankAtk)))
						{
							INCREASE_STATUS_VIABILITY(1);
							break;
						}
						else if (i == STAT_STAGE_SPATK && (SpecialMoveInMoveset(bankAtk)))
						{
							INCREASE_STATUS_VIABILITY(1);
							break;
						}
						else if (i == STAT_STAGE_ACC || i == STAT_STAGE_EVASION || i == STAT_STAGE_SPEED)
						{
							INCREASE_STATUS_VIABILITY(1);
							break;
						}
						else if (IsClassStall(class)) //Defense and Sp. Defense
						{
							INCREASE_STATUS_VIABILITY(1);
							break;
						}
					}
				}
			}
			break;

		case EFFECT_SEMI_INVULNERABLE:
			if (IS_SINGLE_BATTLE && IsClassSweeper(class))
			{
				if (MoveWouldHitFirst(move, bankAtk, bankDef))
				{
					if (gBattleMoves[predictedMove].effect == EFFECT_EXPLOSION
					||  gBattleMoves[predictedMove].effect == EFFECT_PROTECT)
						INCREASE_VIABILITY(3); //Past strongest move
				}
				else if (gBattleMoves[predictedMove].effect == EFFECT_SEMI_INVULNERABLE
				&& !BATTLER_SEMI_INVULNERABLE(bankDef))
					INCREASE_VIABILITY(3); //Past strongest move
			}
			break;

		case EFFECT_DEFENSE_CURL:
			if (MoveEffectInMoveset(EFFECT_ROLLOUT, bankAtk) && !(data->atkStatus2 & STATUS2_DEFENSE_CURL))
				INCREASE_STAT_VIABILITY(STAT_STAGE_DEF, STAT_STAGE_MAX, 5);
			else
				goto AI_DEFENSE_PLUS;
			break;

		case EFFECT_FAKE_OUT:
			if (move == MOVE_FAKEOUT
			&& ShouldUseFakeOut(bankAtk, bankDef))
			{
				IncreaseFakeOutViability(&viability, class, bankAtk, bankDef, move);
			}
			break;

		case EFFECT_STOCKPILE:
			if (MoveEffectInMoveset(EFFECT_SWALLOW, bankAtk)
			||  MoveEffectInMoveset(EFFECT_SPIT_UP, bankAtk))
				INCREASE_STATUS_VIABILITY(2);
			else
				goto AI_COSMIC_POWER;
			break;

		case EFFECT_HAIL:
			if (MoveInMoveset(MOVE_AURORAVEIL, bankAtk))
			{
				if (IsClassScreener(class))
				{
					INCREASE_VIABILITY(8);
					break;
				}
				else if (IsClassDoublesTeamSupport(class))
				{
					INCREASE_VIABILITY(17);
					break;
				}
			}
			else if (IsClassDoublesSetupAttacker(class)
			&& IsTypeZCrystal(data->atkItem, moveType)
			&& !gNewBS->zMoveData.used[bankAtk] //Z-Crystal provides speed up
			&& !IsMegaZMoveBannedBattle()
			&& atkAbility != ABILITY_CONTRARY)
				IncreaseTailwindViability(&viability, class, bankAtk, bankDef);

			else if (IsClassDoublesSetupAttacker(class)
			&& atkAbility == ABILITY_SLUSHRUSH)
				IncreaseTailwindViability(&viability, class, bankAtk, bankDef);

			else if (atkAbility == ABILITY_SNOWCLOAK
			|| atkAbility == ABILITY_ICEBODY
			|| atkAbility == ABILITY_FORECAST
			|| atkAbility == ABILITY_SLUSHRUSH
			|| atkAbility == ABILITY_MAGICGUARD
			|| atkAbility == ABILITY_OVERCOAT
			|| MoveInMoveset(MOVE_BLIZZARD, bankAtk)
			|| MoveInMoveset(MOVE_AURORAVEIL, bankAtk)
			|| MoveInMoveset(MOVE_WEATHERBALL, bankAtk)
			|| MoveEffectInMoveset(EFFECT_MORNING_SUN, bankDef)
			|| data->atkItemEffect == ITEM_EFFECT_ICY_ROCK)
			{
				if (IsClassDoublesTeamSupport(class))
					INCREASE_VIABILITY(17);
				else
					INCREASE_STATUS_VIABILITY(2);
			}
			break;

		case EFFECT_TORMENT:
			INCREASE_STATUS_VIABILITY(0);
			break;

		case EFFECT_WILL_O_WISP:
		AI_BURN_CHECKS:
			if (!BadIdeaToBurn(bankDef, bankAtk))
			{
				if (CalcMoveSplit(bankDef, predictedMove) == SPLIT_PHYSICAL
				&& MoveKnocksOutXHits(predictedMove, bankDef, bankAtk, 1))
					INCREASE_STATUS_VIABILITY(3); //If the enemy can kill with a physical move, try burning them so they can't anymore

				else if ((IsClassDoublesUtility(class) || IsClassDoublesTeamSupport(class))
				&& PhysicalMoveInMoveset(bankDef))
				{
					//They're split up for now so just in case they change
					if (IsClassDoublesUtility(class))
						INCREASE_VIABILITY(11);
					else //(IsClassDoublesTeamSupport(class))
						INCREASE_VIABILITY(11);
				}
				else if (MoveInMoveset(MOVE_HEX, bankAtk)
				|| MoveInMoveset(MOVE_HEX, data->bankAtkPartner)
				|| PhysicalMoveInMoveset(bankDef))
					INCREASE_STATUS_VIABILITY(2);
				else
					INCREASE_STATUS_VIABILITY(1);
			}
			break;

		case EFFECT_MEMENTO:
			//TODO
			break;

		case EFFECT_FOLLOW_ME:
			if (IS_DOUBLE_BATTLE
			&& move != MOVE_SPOTLIGHT
			&& !IsBankIncapacitated(bankDef)
			&& (move != MOVE_RAGEPOWDER || IsAffectedByPowder(bankDef)) //Rage Powder doesn't affect powder immunities
			&& BATTLER_ALIVE(data->bankAtkPartner))
			{
				u16 predictedMoveOnPartner = IsValidMovePrediction(bankDef, data->bankAtkPartner);

				if (SPLIT(predictedMoveOnPartner) != SPLIT_STATUS)
				{
					if (IsClassDoublesUtility(class))
						INCREASE_VIABILITY(16);
					else
						INCREASE_STATUS_VIABILITY(3);
				}
			}
			break;

		case EFFECT_NATURE_POWER:
			return AIScript_Positives(bankAtk, bankDef, GetNaturePowerMove(), originalViability, data);

		case EFFECT_CHARGE:
			for (i = 0; i < MAX_MON_MOVES; ++i)
			{
				if (GetMoveTypeSpecial(bankAtk, gBattleMons[bankAtk].moves[i]) == TYPE_ELECTRIC
				&&  SPLIT(gBattleMons[bankAtk].moves[i]) != SPLIT_STATUS)
					INCREASE_STATUS_VIABILITY(2);
			}

			goto AI_SPECIAL_DEFENSE_PLUS;

		case EFFECT_TAUNT:
			if (SPLIT(predictedMove) == SPLIT_STATUS)
				INCREASE_STATUS_VIABILITY(3);
			else if (StatusMoveInMoveset(bankDef))
				INCREASE_STATUS_VIABILITY(2);
			break;

		case EFFECT_TRICK: //+ Bestwo
			switch (data->atkItemEffect) {
				case ITEM_EFFECT_CHOICE_BAND: ;
					if (data->atkItemQuality == QUALITY_CHOICE_SCARF
					|| !MoveSplitInMoveset(data->atkItemQuality, bankDef)) //Target doesn't have beneficial move for choice item
						INCREASE_STATUS_VIABILITY(2);
					break;

				case ITEM_EFFECT_TOXIC_ORB:
					if (!GoodIdeaToPoisonSelf(bankAtk) && !BadIdeaToPoison(bankDef, bankAtk))
						INCREASE_STATUS_VIABILITY(2);
					break;

				case ITEM_EFFECT_FLAME_ORB:
					if (!GoodIdeaToBurnSelf(bankAtk) && !BadIdeaToBurn(bankDef, bankAtk))
						INCREASE_STATUS_VIABILITY(2);
					break;

				case ITEM_EFFECT_BLACK_SLUDGE:
					if (!IsOfType(bankDef, TYPE_POISON))
						INCREASE_STATUS_VIABILITY(3);
					break;

				case ITEM_EFFECT_IRON_BALL:
					if (!MoveInMoveset(MOVE_FLING, bankDef) || CheckGrounding(bankDef) != GROUNDED)
						INCREASE_STATUS_VIABILITY(2);
					break;

				case ITEM_EFFECT_LAGGING_TAIL:
				case ITEM_EFFECT_STICKY_BARB:
					INCREASE_STATUS_VIABILITY(3);
					break;

				case ITEM_EFFECT_UTILITY_UMBRELLA:
					if (atkAbility != ABILITY_SOLARPOWER && atkAbility != ABILITY_DRYSKIN)
					{
						switch (defAbility) {
							case ABILITY_SWIFTSWIM:
								if (gBattleWeather & WEATHER_RAIN_ANY)
									INCREASE_STATUS_VIABILITY(3); //Slow 'em down
								break;
							case ABILITY_CHLOROPHYLL:
							case ABILITY_FLOWERGIFT:
								if (gBattleWeather & WEATHER_SUN_ANY)
									INCREASE_STATUS_VIABILITY(3); //Slow 'em down
								break;
						}
					}
					break;

				case ITEM_EFFECT_EJECT_BUTTON:
					if (!IsRaidBattle() && IsDynamaxed(bankDef) && gNewBS->dynamaxData.timer[bankDef] > 1
					&& (DamagingMoveInMoveset(bankAtk)
					 || (IS_DOUBLE_BATTLE && BATTLER_ALIVE(data->bankAtkPartner) && DamagingMoveInMoveset(data->bankAtkPartner))))
						INCREASE_STATUS_VIABILITY(2); //Force 'em out next turn
					break;

				default:
					if (move != MOVE_BESTOW && data->atkItem == ITEM_NONE)
					{
						switch (data->defItemEffect) {
							case ITEM_EFFECT_CHOICE_BAND:
								break;

							case ITEM_EFFECT_TOXIC_ORB:
								if (GoodIdeaToPoisonSelf(bankAtk))
									INCREASE_STATUS_VIABILITY(2);
								break;

							case ITEM_EFFECT_FLAME_ORB:
								if (GoodIdeaToBurnSelf(bankAtk))
									INCREASE_STATUS_VIABILITY(2);
								break;

							case ITEM_EFFECT_BLACK_SLUDGE:
								if (IsOfType(bankAtk, TYPE_POISON))
									INCREASE_STATUS_VIABILITY(3);
								break;

							case ITEM_EFFECT_IRON_BALL:
								if (MoveInMoveset(MOVE_FLING, bankAtk))
									INCREASE_STATUS_VIABILITY(2);
								break;

							case ITEM_EFFECT_LAGGING_TAIL:
							case ITEM_EFFECT_STICKY_BARB:
								break;

							default:
								INCREASE_STATUS_VIABILITY(1);
						}
					}
			}
			break;

		case EFFECT_ROLE_PLAY:
			//To do
			break;

		case EFFECT_WISH:
			if (ShouldUseWishAromatherapy(bankAtk, bankDef, move, class))
				INCREASE_VIABILITY(8);
			else
				goto AI_RECOVER;
			break;

		case EFFECT_INGRAIN: //+ Aqua Ring
			if (data->atkItemEffect == ITEM_EFFECT_BIG_ROOT)
				INCREASE_STATUS_VIABILITY(2);
			else
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_SUPERPOWER:
			if (move != MOVE_HYPERSPACEHOLE
			&&  atkAbility != ABILITY_CONTRARY
			&& data->atkItemEffect == ITEM_EFFECT_EJECT_PACK)
				goto PIVOT_CHECK;
			break;

		case EFFECT_MAGIC_COAT:
			if (SPLIT(predictedMove) == SPLIT_STATUS && gBattleMoves[predictedMove].target & (MOVE_TARGET_SELECTED | MOVE_TARGET_OPPONENTS_FIELD | MOVE_TARGET_BOTH))
				INCREASE_STATUS_VIABILITY(3);
			break;

		case EFFECT_RECYCLE:
			//If saved item exists and bank not holding anything
			if (SAVED_CONSUMED_ITEMS(bankAtk) != 0 && data->atkItem == 0)
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_BRICK_BREAK:
			if (IsRaidBattle() && SIDE(bankAtk) == B_SIDE_PLAYER
			&& gNewBS->dynamaxData.raidShieldsUp //Brick Break destroys 2 Raid shields
			&& (gNewBS->dynamaxData.shieldCount - gNewBS->dynamaxData.shieldsDestroyed >= 2) //At least two shields left
			&& !MoveBlockedBySubstitute(move, bankAtk, bankDef)
			&& !(AI_SpecialTypeCalc(move, bankAtk, bankDef) & MOVE_RESULT_DOESNT_AFFECT_FOE))
			{
				INCREASE_VIABILITY(18); //Super important
			}
			else if (gSideStatuses[SIDE(bankDef)] & (SIDE_STATUS_REFLECT | SIDE_STATUS_LIGHTSCREEN)
			|| gNewBS->AuroraVeilTimers[SIDE(bankDef)] > 0)
			{
				if (IsClassSweeper(class))
					INCREASE_VIABILITY(3); //Increase past strongest move
				else
					INCREASE_STATUS_VIABILITY(2);
			}
			break;

		case EFFECT_KNOCK_OFF:
			if (CanKnockOffItem(bankDef))
			{
				switch (data->defItemEffect) {
					case ITEM_EFFECT_IRON_BALL:
					case ITEM_EFFECT_LAGGING_TAIL:
					case ITEM_EFFECT_STICKY_BARB:
						break;

					default:
						INCREASE_VIABILITY(3); //Increase past strongest move
				}
			}
			break;

		case EFFECT_SKILL_SWAP:
			switch (move) {
				case MOVE_WORRYSEED:
				case MOVE_GASTROACID:
				case MOVE_SIMPLEBEAM:
					if (gAbilityRatings[defAbility] >= 5)
						INCREASE_STATUS_VIABILITY(2);
					break;

				case MOVE_ENTRAINMENT:
					if (gAbilityRatings[defAbility] >= 5
					||  gAbilityRatings[atkAbility] <= 0)
					{
						if (defAbility != atkAbility
						&& !IsAbilitySuppressed(bankDef))
							INCREASE_STATUS_VIABILITY(2);
					}						
					break;

				case MOVE_SKILLSWAP:
					if (gAbilityRatings[defAbility] > gAbilityRatings[atkAbility])
						INCREASE_STATUS_VIABILITY(1);
					break;
			}
			break;

		case EFFECT_IMPRISON:
			if (MoveInMoveset(predictedMove, bankAtk)) //The attacker knows the move the opponent is about to use
				INCREASE_STATUS_VIABILITY(3);
			else
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_REFRESH:
			switch (move) {
				case MOVE_PSYCHOSHIFT:
					if (data->atkStatus1 & STATUS1_PSN_ANY)
						goto AI_POISON_CHECKS;
					else if (data->atkStatus1 & STATUS1_BURN)
						goto AI_BURN_CHECKS;
					else if (data->atkStatus1 & STATUS1_PARALYSIS)
						goto AI_PARALYZE_CHECKS;
					else if (data->atkStatus1 & STATUS1_SLEEP)
						goto AI_SLEEP_CHECKS;
					break;

				case MOVE_REFRESH:
					if (data->atkStatus1 & STATUS1_ANY)
						INCREASE_STATUS_VIABILITY(3);
					break;
			}
			break;

		case EFFECT_GRUDGE:
			break;

		case EFFECT_SNATCH:
			if (gBattleMoves[predictedMove].flags & FLAG_SNATCH_AFFECTED)
				INCREASE_STATUS_VIABILITY(3); //Steal move
			break;

		case EFFECT_MUD_SPORT:
			if (!MoveTypeInMoveset(TYPE_ELECTRIC, bankAtk) && MoveTypeInMoveset(TYPE_ELECTRIC, bankDef))
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_TICKLE:
			if (STAT_STAGE(bankDef, STAT_STAGE_DEF) > 4 && PhysicalMoveInMoveset(bankAtk) && defAbility != ABILITY_CONTRARY)
				goto AI_DEFENSE_MINUS;
			else
				goto AI_ATTACK_MINUS;
			break;

		case EFFECT_COSMIC_POWER:
		AI_COSMIC_POWER: ;
			if (atkAbility != ABILITY_CONTRARY)
			{
				if (STAT_STAGE(bankAtk, STAT_STAGE_DEF) < 10 || IsClassBatonPass(class))
					goto AI_DEFENSE_PLUS;
				else
					goto AI_SPECIAL_DEFENSE_PLUS;
			}
			break;

		case EFFECT_EXTREME_EVOBOOST: ;
			u8 oldViability = viability;
			
			switch (move) {
				case MOVE_EXTREME_EVOBOOST:
					for (i = STAT_STAGE_ATK; i <= STAT_STAGE_SPDEF; ++i)
					{
						INCREASE_STAT_VIABILITY(i, STAT_STAGE_MAX, 3);
						if (viability != oldViability) //Viability was increased
							break;
					}
					break;
				case MOVE_CLANGOROUSSOUL:
					if (MoveKnocksOutXHits(predictedMove, bankDef, bankAtk, 1))
						break;
					else if (MoveKnocksOutXHits(predictedMove, bankDef, bankAtk, 3)
					&& !MoveKnocksOutXHits(predictedMove, bankDef, bankAtk, 2))
						break; //Will be able to KO after the HP cut
					//Fallthrough
				case MOVE_NORETREAT:
					if (IsMovePredictionPhazingMove(bankDef, bankAtk) || atkAbility == ABILITY_CONTRARY)
						break;

					//Try to boost either Attack, Sp. Attack, or Speed
					if (PhysicalMoveInMoveset(bankAtk))
					{
						INCREASE_STAT_VIABILITY(STAT_STAGE_ATK, 8, 2);
						if (viability != oldViability) //Viability was increased
							break;
					}

					if (SpecialMoveInMoveset(bankAtk))
					{
						INCREASE_STAT_VIABILITY(STAT_STAGE_SPATK, 8, 2);
						if (viability != oldViability) //Viability was increased
							break;
					}

					INCREASE_STAT_VIABILITY(STAT_STAGE_SPEED, 8, 3); //At least try to boost Speed
					break;
			}
			break;

		case EFFECT_BULK_UP:
			if (atkAbility != ABILITY_CONTRARY)
			{
				if (STAT_STAGE(bankAtk, STAT_STAGE_ATK) < 8 || IsClassBatonPass(class))
					goto AI_ATTACK_PLUS;
				else if (STAT_STAGE(bankAtk, STAT_STAGE_DEF) < 8 || IsClassBatonPass(class)) //Normally checks for 10 Def
					goto AI_DEFENSE_PLUS;
			}
			break;

		case EFFECT_WATER_SPORT:
			if (!MoveTypeInMoveset(TYPE_FIRE, bankAtk) && (MoveTypeInMoveset(TYPE_FIRE, bankDef)))
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_CALM_MIND:
			if (atkAbility != ABILITY_CONTRARY)
			{
				switch (move) {
					case MOVE_GEOMANCY:
						if (data->atkItemEffect == ITEM_EFFECT_POWER_HERB)
							INCREASE_STAT_VIABILITY(STAT_STAGE_SPEED, STAT_STAGE_MAX, 3);
						__attribute__ ((fallthrough));

					case MOVE_QUIVERDANCE:
						if (SpeedCalc(bankAtk) <= SpeedCalc(bankDef) || IsClassBatonPass(class))
							goto AI_SPEED_PLUS;
						__attribute__ ((fallthrough));

					default:
						if (STAT_STAGE(bankAtk, STAT_STAGE_SPATK) < 8 || IsClassBatonPass(class))
							goto AI_SPECIAL_ATTACK_PLUS;
						else if (STAT_STAGE(bankAtk, STAT_STAGE_SPDEF) < 8 || IsClassBatonPass(class)) //Normally checks for 10 Sp. Def
							goto AI_SPECIAL_DEFENSE_PLUS;
				}
			}
			break;

		case EFFECT_DRAGON_DANCE:
			if (atkAbility != ABILITY_CONTRARY)
			{
				switch (move) {
					case MOVE_SHELLSMASH:
						if (data->atkItemEffect == ITEM_EFFECT_POWER_HERB)
							INCREASE_STAT_VIABILITY(STAT_STAGE_SPEED, STAT_STAGE_MAX, 3);
						else if (SpeedCalc(bankAtk) <= SpeedCalc(bankDef) || IsClassBatonPass(class))
							goto AI_SPEED_PLUS;
						else if ((IsClassBatonPass(class) && STAT_STAGE(bankAtk, STAT_STAGE_SPATK) < 10)
						|| (STAT_STAGE(bankAtk, STAT_STAGE_SPATK) < 8 && (SpecialMoveInMoveset(bankAtk))))
							goto AI_SPECIAL_ATTACK_PLUS;
						else
							goto AI_ATTACK_PLUS;
						__attribute__ ((fallthrough));

					default: //Dragon Dance + Shift Gear
						if (SpeedCalc(bankAtk) <= SpeedCalc(bankDef) || IsClassBatonPass(class))
							goto AI_SPEED_PLUS;
						else
							goto AI_ATTACK_PLUS;
						break;
				}
			}
			break;

		case EFFECT_STAT_SWAP_SPLIT:
			switch (move)
			{
				case MOVE_GUARDSWAP: //Prevent's getting caught in an infinite loop
					if (STAT_STAGE(bankDef, STAT_STAGE_DEF) > STAT_STAGE(bankAtk, STAT_STAGE_DEF)
					&&  STAT_STAGE(bankDef, STAT_STAGE_SPDEF) >= STAT_STAGE(bankAtk, STAT_STAGE_SPDEF))
						INCREASE_STATUS_VIABILITY(1);
					else if (STAT_STAGE(bankDef, STAT_STAGE_SPDEF) > STAT_STAGE(bankAtk, STAT_STAGE_SPDEF)
					&&  STAT_STAGE(bankDef, STAT_STAGE_DEF) >= STAT_STAGE(bankAtk, STAT_STAGE_DEF))
						INCREASE_STATUS_VIABILITY(1);
					break;

				case MOVE_POWERSWAP:
					if (STAT_STAGE(bankDef, STAT_STAGE_ATK) > STAT_STAGE(bankAtk, STAT_STAGE_ATK)
					&&  STAT_STAGE(bankDef, STAT_STAGE_SPATK) >= STAT_STAGE(bankAtk, STAT_STAGE_SPATK))
						INCREASE_STATUS_VIABILITY(1);
					else if (STAT_STAGE(bankDef, STAT_STAGE_SPATK) > STAT_STAGE(bankAtk, STAT_STAGE_SPATK)
					&&  STAT_STAGE(bankDef, STAT_STAGE_ATK) >= STAT_STAGE(bankAtk, STAT_STAGE_ATK))
						INCREASE_STATUS_VIABILITY(1);
					break;

				case MOVE_POWERTRICK:
					if (!(data->atkStatus3 & STATUS3_POWER_TRICK))
					{
						if (data->atkDefense > data->atkAttack && PhysicalMoveInMoveset(bankAtk))
							INCREASE_STATUS_VIABILITY(2);
						break;
					}
					break;

				case MOVE_HEARTSWAP: ;
					bool8 hasHigherStat = FALSE;

					//Only use if all target stats are >= attacker stats to prevent infinite loop
					for (i = STAT_STAGE_ATK; i < BATTLE_STATS_NO; ++i)
					{
						if (STAT_STAGE(bankDef, i) < STAT_STAGE(bankAtk, i))
							break;

						if (STAT_STAGE(bankDef, i) > STAT_STAGE(bankAtk, i))
							hasHigherStat = TRUE;
					}

					if (hasHigherStat && i == BATTLE_STATS_NO)
						INCREASE_STATUS_VIABILITY(1);
					break;

				case MOVE_SPEEDSWAP:
					if (data->defSpeed > data->atkSpeed)
						INCREASE_STATUS_VIABILITY(3);
					break;

				case MOVE_GUARDSPLIT: ;
					u16 newDefense = (data->atkDefense + data->defDefense) / 2;
					u16 newSpDef = (data->atkSpDef + data->defSpDef) / 2;

					if ((newDefense > data->atkDefense && newSpDef >= data->atkSpDef)
					|| (newSpDef > data->atkSpDef && newDefense >= data->atkDefense))
						INCREASE_STATUS_VIABILITY(1);
					break;

				case MOVE_POWERSPLIT: ;
					u16 newAttack = (data->atkAttack + data->defAttack) / 2;
					u16 newSpAtk = (data->atkSpAtk + data->defSpAtk) / 2;

					if ((newAttack > data->atkAttack && newSpAtk >= data->atkSpAtk)
					|| (newSpAtk > data->atkSpAtk && newAttack >= data->atkAttack))
						INCREASE_STATUS_VIABILITY(1);
					break;
			}
			break;

		case EFFECT_EAT_BERRY:
			switch (move)
			{
				case MOVE_BUGBITE:
				case MOVE_PLUCK:
					if (data->defStatus2 & STATUS2_SUBSTITUTE
					||  defAbility == ABILITY_STICKYHOLD)
						break;
					else if (IsBerry(data->defItem))
						INCREASE_VIABILITY(3); //Increase past strongest move
					break;

				case MOVE_INCINERATE:
					if (data->defStatus2 & STATUS2_SUBSTITUTE
					||  defAbility == ABILITY_STICKYHOLD)
						break;
					else if (IsBerry(data->defItem) || IsGem(data->defItem))
						INCREASE_VIABILITY(3); //Increase past strongest move
					break;
			}
			break;

		case EFFECT_SMACK_DOWN:
			if (CheckGrounding(bankDef) != GROUNDED)
				INCREASE_VIABILITY(3); //Increase past strongest move
			break;

		case EFFECT_REMOVE_TARGET_STAT_CHANGES:
			if (ShouldPhaze(bankAtk, bankDef, move, class))
				INCREASE_VIABILITY(8);
			break;

		case EFFECT_RELIC_SONG:
			#if (defined SPECIES_MELOETTA && defined SPECIES_MELOETTA_PIROUETTE)
			if (data->atkSpecies == SPECIES_MELOETTA && data->defDefense < data->defSpDef) //Change to pirouette if can do more damage
				INCREASE_VIABILITY(3); //Increase past strongest move
			else if (data->atkSpecies == SPECIES_MELOETTA_PIROUETTE && data->defSpDef < data->defDefense) //Change to Aria if can do more damage
				INCREASE_VIABILITY(3); //Increase past strongest move
			#endif
//			else
//				goto AI_SLEEP_CHECKS;
			break;

		case EFFECT_SET_TERRAIN:
			if (data->atkStatus3 & STATUS3_YAWN && CheckGrounding(bankAtk))
			{
				switch (move) {
					case MOVE_ELECTRICTERRAIN:
					case MOVE_MISTYTERRAIN:
					case MOVE_MAX_LIGHTNING_P:
					case MOVE_MAX_LIGHTNING_S:
					case MOVE_MAX_STARFALL_P:
					case MOVE_MAX_STARFALL_S:
						//Stop yourself from falling asleep
						IncreaseFakeOutViability(&viability, class, bankAtk, bankDef, move); //Treat very important
						break;
				}
			}

			INCREASE_STATUS_VIABILITY(2);
		break;

		case EFFECT_PLEDGE:
			if (IS_DOUBLE_BATTLE)
			{
				if (MoveEffectInMoveset(EFFECT_PLEDGE, data->bankAtkPartner))
					INCREASE_VIABILITY(3); //Increase past strongest move if partner might use pledge move
			}
			break;

		case EFFECT_FIELD_EFFECTS:
			switch (move) {
				case MOVE_TRICKROOM:
					if (!IsTrickRoomActive()
					&& GetPokemonOnSideSpeedAverage(bankAtk) < GetPokemonOnSideSpeedAverage(bankDef))
					{
						if (IsClassDoublesTrickRoomer(class))
							INCREASE_VIABILITY(19);
						else
							INCREASE_STATUS_VIABILITY(3);
					}
					else if (IsTrickRoomActive()
					&& GetPokemonOnSideSpeedAverage(bankAtk) >= GetPokemonOnSideSpeedAverage(bankDef))
					{
						if (IsClassDoublesTrickRoomer(class))
							INCREASE_VIABILITY(19);
						else
							INCREASE_STATUS_VIABILITY(3);
					}
					break;

				case MOVE_MAGICROOM:
					INCREASE_STATUS_VIABILITY(1);
					break;

				case MOVE_WONDERROOM:
					if ((PhysicalMoveInMoveset(bankDef) && data->atkDefense < data->atkSpDef)
					||  (SpecialMoveInMoveset(bankDef) && data->atkSpDef < data->atkDefense))
						INCREASE_STATUS_VIABILITY(2);
					break;

				case MOVE_GRAVITY:
				case MOVE_G_MAX_GRAVITAS_P:
				case MOVE_G_MAX_GRAVITAS_S:
					if (!IsGravityActive())
					{
						if (SleepMoveInMovesetWithLowAccuracy(bankAtk, bankDef)) //Has Gravity for a move like Hypnosis
							IncreaseSleepViability(&viability, class, bankAtk, bankDef, move);
						else if (MoveInMovesetWithAccuracyLessThan(bankAtk, bankDef, 90, FALSE))
							INCREASE_STATUS_VIABILITY(2);
						else
							INCREASE_STATUS_VIABILITY(1);
					}
					break;

				case MOVE_IONDELUGE:
					if ((atkAbility == ABILITY_VOLTABSORB
					||  atkAbility == ABILITY_MOTORDRIVE
					||  atkAbility == ABILITY_LIGHTNINGROD)
					 && GetMoveTypeSpecial(bankDef, predictedMove) == TYPE_NORMAL)
						INCREASE_STATUS_VIABILITY(2);
					break;
			}
			break;

		case EFFECT_FLING: ;
			u8 effect = gFlingTable[data->atkItem].effect;

			switch (effect) {
				case MOVE_EFFECT_BURN:
					goto AI_BURN_CHECKS;

				case MOVE_EFFECT_FLINCH:
					goto AI_FLINCH_CHECKS;

				case MOVE_EFFECT_PARALYSIS:
					goto AI_PARALYZE_CHECKS;

				case MOVE_EFFECT_POISON:
				case MOVE_EFFECT_TOXIC:
					goto AI_POISON_CHECKS;

				case MOVE_EFFECT_FREEZE:
					if (!BadIdeaToFreeze(bankDef, bankAtk))
						INCREASE_STATUS_VIABILITY(3); //Freeze the sucker
					break;
			}
			break;

		case EFFECT_FEINT:
			if (gBattleMoves[predictedMove].effect == EFFECT_PROTECT)
				INCREASE_VIABILITY(3);
			break;

		case EFFECT_ATTACK_BLOCKERS:
			switch (move) {
				case MOVE_EMBARGO:
					if (data->defItemEffect != 0)
						INCREASE_STATUS_VIABILITY(1);
					break;

				case MOVE_POWDER:
					if (SPLIT(predictedMove) != SPLIT_STATUS
					&& GetMoveTypeSpecial(bankDef, predictedMove) == TYPE_FIRE)
						INCREASE_STATUS_VIABILITY(3);
					break;

				case MOVE_TELEKINESIS:
					if (MoveInMovesetWithAccuracyLessThan(bankAtk, bankDef, 90, FALSE))
						INCREASE_STATUS_VIABILITY(1);
					break;

				case MOVE_THROATCHOP:
					if (MoveWouldHitFirst(move, bankAtk, bankDef) && CheckSoundMove(predictedMove))
						INCREASE_STATUS_VIABILITY(3);
					else if (IS_DOUBLE_BATTLE)
						IncreaseDoublesDamageViabilityToScore(&viability, class, 5, bankAtk, bankDef);
					else if (IsClassSweeper(class) && SoundMoveInMoveset(bankDef))
						INCREASE_VIABILITY(3); //Past strongest move
					break;

				default: //Heal Block
					if (MoveWouldHitFirst(move, bankAtk, bankDef) && IsMovePredictionHealingMove(bankDef, bankAtk))
						INCREASE_STATUS_VIABILITY(3); //Try to cancel move

					else if (HealingMoveInMoveset(bankDef)
					|| data->defItemEffect == ITEM_EFFECT_LEFTOVERS
					|| (data->defItemEffect == ITEM_EFFECT_BLACK_SLUDGE && IsOfType(bankDef, TYPE_POISON)))
						INCREASE_STATUS_VIABILITY(2);
			}
			break;

		case EFFECT_TYPE_CHANGES:
			switch (move) {
				case MOVE_SOAK:
					if (IsClassSweeper(class)
					&& (MoveTypeInMoveset(bankAtk, TYPE_ELECTRIC) || MoveTypeInMoveset(bankAtk, TYPE_GRASS)))
						INCREASE_STATUS_VIABILITY(2); //Get some super effective moves
					break;

				case MOVE_TRICKORTREAT:
				case MOVE_FORESTSCURSE:
					if (defAbility == ABILITY_WONDERGUARD)
						INCREASE_STATUS_VIABILITY(2); //Give it more weaknesses
					break;
			}
			break;

		//case EFFECT_HEAL_TARGET:
			//Refer to AI_Partner.c
			//break;

		case EFFECT_TOPSY_TURVY_ELECTRIFY:
			switch (move) {
				case MOVE_ELECTRIFY:
					if (GetMoveTypeSpecial(bankDef, predictedMove) == TYPE_NORMAL
					&& (atkAbility == ABILITY_VOLTABSORB
					 || atkAbility == ABILITY_MOTORDRIVE
					 || atkAbility == ABILITY_LIGHTNINGROD))
					{
						INCREASE_STATUS_VIABILITY(3);
					}
					break;

				default: ; //Topsy Turvy
					u8 targetPositiveStages = CountBanksPositiveStatStages(bankDef);
					u8 targetNegativeStages = CountBanksNegativeStatStages(bankDef);

					if (targetPositiveStages > targetNegativeStages)
						INCREASE_STATUS_VIABILITY(1);
			}
			break;

		case EFFECT_FAIRY_LOCK_HAPPY_HOUR:
			switch (move) {
				case MOVE_FAIRYLOCK:
					if (!IsTrapped(bankDef, TRUE))
					{
						if (ShouldTrap(bankAtk, bankDef, move, class))
							INCREASE_VIABILITY(8);
					}
					break;

				case MOVE_CELEBRATE:
				case MOVE_HOLDHANDS:
				case MOVE_HAPPYHOUR:
					if (IsTypeZCrystal(data->atkItem, moveType) && !IsMegaZMoveBannedBattle() && !gNewBS->zMoveData.used[bankAtk])
						INCREASE_VIABILITY(9); //Z-Happy Hour! / Z-Celebrate
					break;
			}
			break;

		case EFFECT_INSTRUCT_AFTER_YOU_QUASH:
			if (IS_DOUBLE_BATTLE)
			{
				switch (move) {
					case MOVE_QUASH:
						if (!MoveWouldHitFirst(move, data->bankAtkPartner, bankDef)) //Attacker partner wouldn't go before target
						{
							if (IsClassDoublesTeamSupport(class))
								INCREASE_VIABILITY(7);
							else
								INCREASE_STATUS_VIABILITY(1);
						}
						break;
				}
			}
		break;

		case EFFECT_TEAM_EFFECTS:
			switch (move)
			{
				case MOVE_TAILWIND:
					IncreaseTailwindViability(&viability, class, bankAtk, bankDef);
					break;

				case MOVE_LUCKYCHANT:
					if (IS_SINGLE_BATTLE)
						INCREASE_STATUS_VIABILITY(1);
					else
					{
						if (IsClassDoublesTeamSupport(class) && ViableMonCountFromBank(bankDef) > 1) //Just try to kill last foe
							INCREASE_VIABILITY(8);
					}
					break;

				case MOVE_MAGNETRISE:
					if (CheckGrounding(bankAtk) == GROUNDED
					&& DamagingMoveTypeInMoveset(bankDef, TYPE_GROUND)
					&& !(AI_SpecialTypeCalc(MOVE_EARTHQUAKE, bankDef, bankAtk) & (MOVE_RESULT_DOESNT_AFFECT_FOE | MOVE_RESULT_NOT_VERY_EFFECTIVE))) //Doesn't resist ground move
					{
						if (MoveWouldHitFirst(move, bankAtk, bankDef) //Attacker goes first
						&&  GetMoveTypeSpecial(bankDef, predictedMove) == TYPE_GROUND)
						{
							INCREASE_STATUS_VIABILITY(3); //Cause the enemy's move to fail
							break;
						}
						else //Opponent Goes First
						{
							if (GetMoveTypeSpecial(bankDef, predictedMove) == TYPE_GROUND)
								INCREASE_STATUS_VIABILITY(2); //If he's using it now, he'll probably use it again later
							else
								INCREASE_STATUS_VIABILITY(1);
							break;
						}
					}
					break;
			}
			break;

		case EFFECT_CAMOUFLAGE:
			if (MoveWouldHitFirst(move, bankAtk, bankDef)
			&& moveSplit != SPLIT_STATUS
			&& !(AI_SpecialTypeCalc(predictedMove, bankDef, bankAtk) & MOVE_RESULT_DOESNT_AFFECT_FOE))
				INCREASE_STATUS_VIABILITY(1);
			break;

		case EFFECT_FLAMEBURST:
			if (IS_DOUBLE_BATTLE)
			{
				if (BATTLER_ALIVE(data->bankDefPartner)
				&& GetHealthPercentage(data->bankDefPartner) < 12
				&& ABILITY(data->bankDefPartner) != ABILITY_MAGICGUARD
				&& !IsOfType(data->bankDefPartner, TYPE_FIRE))
					IncreaseDoublesDamageViabilityToScore(&viability, class, 10, bankAtk, bankDef);
			}
			break;

		case EFFECT_LASTRESORT_SKYDROP:
			if (move == MOVE_SKYDROP)
			{
				if (IS_SINGLE_BATTLE)
				{
					if (IsClassSweeper(class)
					&& IsTakingSecondaryDamage(bankDef))
						INCREASE_VIABILITY(3); //Past strongest move
				}
				else //Double Battle
				{
					if (IsTakingSecondaryDamage(bankDef))
						IncreaseDoublesDamageViabilityToScore(&viability, class, 5, bankAtk, bankDef);
				}
			}
			break;
	}

	if (moveSplit != SPLIT_STATUS)
	{
		if (IS_SINGLE_BATTLE) //Single Battle or only 1 target left
		{
			//Every spread type has the same viability increases for these two
			if (MoveKnocksOutPossiblyGoesFirstWithBestAccuracy(move, bankAtk, bankDef, TRUE) //Check Going First
			&& (AccuracyCalc(move, bankAtk, bankDef) >= 70 //If the AI's best killing move has a low accuracy, then
			 || !MoveThatCanHelpAttacksHitInMoveset(bankAtk) //try to make it's chance of hitting higher.
			 || CanKnockOut(bankDef, bankAtk))) //Just use the move if you'll die anyways
			{
				INCREASE_VIABILITY(9);
			}
			else if (!MoveEffectInMoveset(EFFECT_PROTECT, bankAtk)
			&& !MoveWouldHitFirst(move, bankAtk, bankDef) //Attacker wouldn't hit first
			&& Can2HKO(bankDef, bankAtk) //Foe can kill attacker in at least two hits
			&& MoveKnocksOutPossiblyGoesFirstWithBestAccuracy(move, bankAtk, bankDef, FALSE)) //Don't check going first
			{
				INCREASE_VIABILITY(8); //Use the killing move with the best accuracy
			}
			else if (!MoveEffectInMoveset(EFFECT_PROTECT, bankAtk)
			&& MoveKnocksOutXHits(predictedMove, bankDef, bankAtk, 1) //Foe can kill attacker
			&& StrongestMoveGoesFirst(move, bankAtk, bankDef) //Use strongest fast move
			&& (!MoveInMoveset(MOVE_FAKEOUT, bankAtk) || !ShouldUseFakeOut(bankAtk, bankDef))) //Prefer Fake Out if it'll do something
			{
				INCREASE_VIABILITY(9);
			}
			else if (IsStrongestMove(move, bankAtk, bankDef))
			{
				//If the attacker is slower than the target and the target is going to die
				//anyways, then do something else and let it die.
				if (MoveWouldHitFirst(move, bankAtk, bankDef)
				|| !WillFaintFromSecondaryDamage(bankDef)
				|| IsMovePredictionHealingMove(bankDef, bankAtk)
				|| atkAbility == ABILITY_MOXIE
				|| atkAbility == ABILITY_BEASTBOOST)
					INCREASE_VIABILITY(2);
			}
		}
		else //Double Battle
		{
			IncreaseDoublesDamageViability(&viability, class, bankAtk, bankDef, move);
		}
	}

	if (data->atkStatus1 & STATUS1_FREEZE && CheckTableForMove(move, gMovesCanUnfreezeAttacker))
	{
		//Unfreeze yourself
		if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
			INCREASE_VIABILITY(20);
		else
			INCREASE_VIABILITY(10);
	}

	return MathMin(viability, 255);
}

//An AI script meant for generic Trainers who have no set strategy built into their team
u8 AIScript_SemiSmart(const u8 bankAtk, const u8 bankDef, const u16 originalMove, const u8 originalViability, struct AIScript* data)
{
	if (!(AI_THINKING_STRUCT->aiFlags & AI_SCRIPT_CHECK_GOOD_MOVE)) //Semi smart only works if the AI isn't already smart
	{
		u16 move = TryReplaceMoveWithZMove(bankAtk, bankDef, originalMove);
		s16 viability = originalViability;

		if (IS_DOUBLE_BATTLE && TARGETING_PARTNER)
			return AIScript_Partner(bankAtk, data->bankAtkPartner, originalMove, originalViability, data);

		switch (gBattleMoves[move].effect) {
			case EFFECT_HIT:
			case EFFECT_SLEEP:
			case EFFECT_YAWN:
			case EFFECT_ABSORB:
			case EFFECT_DREAM_EATER:
			case EFFECT_POISON_HIT:
			case EFFECT_BURN_HIT:
			case EFFECT_FREEZE_HIT:
			case EFFECT_PARALYZE_HIT:
			case EFFECT_BAD_POISON_HIT:
			case EFFECT_EXPLOSION:
			case EFFECT_MIRROR_MOVE:
			case EFFECT_ATTACK_UP:
			case EFFECT_ATTACK_UP_2:
			case EFFECT_SPEED_UP:
			case EFFECT_SPEED_UP_2:
			case EFFECT_SPECIAL_ATTACK_UP:
			case EFFECT_SPECIAL_ATTACK_UP_2:
			case EFFECT_MULTI_HIT:
			case EFFECT_TRIPLE_KICK:
			case EFFECT_DOUBLE_HIT:
			case EFFECT_FLINCH_HIT:
			case EFFECT_RESTORE_HP:
			case EFFECT_MORNING_SUN:
			case EFFECT_SWALLOW:
			case EFFECT_TOXIC:
			case EFFECT_POISON:
			case EFFECT_REST:
			case EFFECT_CONFUSE:
			case EFFECT_PARALYZE:
			case EFFECT_CONFUSE_HIT:
			case EFFECT_SUBSTITUTE:
			case EFFECT_LEECH_SEED:
			case EFFECT_SPLASH:
			case EFFECT_ENCORE:
			case EFFECT_PAIN_SPLIT:
			case EFFECT_SNORE:
			case EFFECT_SLEEP_TALK:
			case EFFECT_DESTINY_BOND:
			case EFFECT_NIGHTMARE:
			case EFFECT_PROTECT:
			case EFFECT_PERISH_SONG:
			case EFFECT_ROLLOUT:
			case EFFECT_SWAGGER:
			case EFFECT_FLATTER:
			case EFFECT_FURY_CUTTER:
			case EFFECT_ATTRACT:
			case EFFECT_BATON_PASS:
			case EFFECT_ATTACK_UP_HIT:
			case EFFECT_BELLY_DRUM:
			case EFFECT_SEMI_INVULNERABLE:
			case EFFECT_DEFENSE_CURL:
			case EFFECT_FAKE_OUT:
			case EFFECT_STOCKPILE:
			case EFFECT_WILL_O_WISP:
			case EFFECT_FOLLOW_ME:
			case EFFECT_NATURE_POWER:
			case EFFECT_WISH:
			case EFFECT_MAGIC_COAT:
			case EFFECT_BRICK_BREAK:
			case EFFECT_KNOCK_OFF:
			case EFFECT_EXTREME_EVOBOOST:
			case EFFECT_BULK_UP:
			case EFFECT_CALM_MIND:
			case EFFECT_DRAGON_DANCE:
			case EFFECT_RELIC_SONG:
			case EFFECT_DAMAGE_SET_TERRAIN:
			case EFFECT_PLEDGE:
			case EFFECT_FEINT:
				return AIScript_Positives(bankAtk, bankDef, originalMove, originalViability, data);
		}

		//Copied from above
		if (SPLIT(move) != SPLIT_STATUS)
		{
			u16 predictedMove = IsValidMovePrediction(bankDef, bankAtk); //The move the target is likely to make against the attacker

			//Every spread type has the same viability increases for these two
			if (MoveKnocksOutPossiblyGoesFirstWithBestAccuracy(move, bankAtk, bankDef, TRUE) //Check Going First
			&& (AccuracyCalc(move, bankAtk, bankDef) >= 70 //If the AI's best killing move has a low accuracy, then
			 || !MoveThatCanHelpAttacksHitInMoveset(bankAtk) //try to make it's chance of hitting higher.
			 || CanKnockOut(bankDef, bankAtk))) //Just use the move if you'll die anyways
			{
				INCREASE_VIABILITY(9);
			}
			else if (!MoveEffectInMoveset(EFFECT_PROTECT, bankAtk)
			&& !MoveWouldHitFirst(move, bankAtk, bankDef) //Attacker wouldn't hit first
			&& Can2HKO(bankDef, bankAtk) //Foe can kill attacker in at least two hits
			&& MoveKnocksOutPossiblyGoesFirstWithBestAccuracy(move, bankAtk, bankDef, FALSE)) //Don't check going first
			{
				INCREASE_VIABILITY(8); //Use the killing move with the best accuracy
			}
			else if (!MoveEffectInMoveset(EFFECT_PROTECT, bankAtk)
			&& MoveKnocksOutXHits(predictedMove, bankDef, bankAtk, 1) //Foe can kill attacker
			&& StrongestMoveGoesFirst(move, bankAtk, bankDef) //Use strongest fast move
			&& (!MoveInMoveset(MOVE_FAKEOUT, bankAtk) || !ShouldUseFakeOut(bankAtk, bankDef))) //Prefer Fake Out if it'll do something
			{
				INCREASE_VIABILITY(9);
			}
			else if (IsStrongestMove(move, bankAtk, bankDef))
			{
				//If the attacker is slower than the target and the target is going to die
				//anyways, then do something else and let it die.
				if (MoveWouldHitFirst(move, bankAtk, bankDef)
				|| !WillFaintFromSecondaryDamage(bankDef)
				|| IsMovePredictionHealingMove(bankDef, bankAtk)
				|| data->atkAbility == ABILITY_MOXIE
				|| data->atkAbility == ABILITY_BEASTBOOST)
					INCREASE_VIABILITY(2);
			}
		}

		if (data->atkStatus1 & STATUS1_FREEZE && CheckTableForMove(move, gMovesCanUnfreezeAttacker))
		{
			//Unfreeze yourself
			if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
				INCREASE_VIABILITY(20);
			else
				INCREASE_VIABILITY(10);
		}
		
		return viability;
	}

	return originalViability; //No change if not one of the above listed move effects
}
