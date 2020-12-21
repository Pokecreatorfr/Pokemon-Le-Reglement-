#pragma once

/**
 * \file move_battle_scripts.h
 * \brief Contains declarations for battle scripts usually called during the
 *		  execution of attacks.
 */

extern const u8* gBattleScriptsForMoveEffects[];

//Scripts
#define BattleScript_FlushMessageBox (u8*) 0x81D96A8
#define BattleScript_MoveFaint (u8*) 0x81D6947
#define BattleScript_MoveEnd (u8*) 0x81D694E
#define BattleScript_ButItDoesntAffect (u8*) 0x81D7E04
#define BattleScript_Pausex20 (u8*) 0x81D89F1
#define BattleScript_MoveMissedPause (u8*) 0x81D695E
#define BattleScript_HitFromCritCalc (u8*) 0x81D6930
#define BattleScript_AlreadyAtFullHp (u8*) 0x81D7DD1
#define BattleScript_PresentHealTarget (u8*) 0x81D7DB7
#define BattleScript_DestinyBondTakesLife (u8*) 0x81D8C6C
#define BattleScript_GrudgeTakesPp (u8*) 0x81D8FA3
#define BattleScript_RestCantSleep (u8*) 0x81D6EB2
#define BattleScript_ResultMessage (u8*) 0x81D6942

#define BattleScript_SubstituteFade (u8*)  0x81D9526
#define BattleScript_HangedOnMsg (u8*) 0x81D9641
#define BattleScript_OneHitKOMsg (u8*) 0x81D8FE4
#define BattleScript_PrintPayDayMoneyString (u8*) 0x81D911D
#define BattleScript_WrapFree (u8*) 0x81D8DF3
#define BattleScript_LeechSeedFree (u8*) 0x81D8E04
#define BattleScript_SpikesFree (u8*) 0x81D8E0B

#define BattleScript_AllStatsUp (u8*) 0x81D8D55
#define BattleScript_RapidSpinAway (u8*) 0x81D8DF1
#define BattleScript_TargetPRLZHeal (u8*) 0x81D9177
#define BattleScript_AtkDefDown (u8*) 0x81D8F35
#define BattleScript_StatUp (u8*) 0x81D6BD1
#define BattleScript_StatDown (u8*) 0x81D6C62
#define BattleScript_SAtkDown2 (u8*) 0x81D8FEB
#define ScreensShatteredString (u8*) 0x83FC646

extern const u8 BattleScript_PauseResultMessage[];
extern const u8 BattleScript_MagneticFluxStatBoost[];
extern const u8 BattleScript_GearUpStatBoost[];
extern const u8 BattleScript_SapSipperAromatherapy[];
extern const u8 BattleScript_DefogAdditionalEffects[];
extern const u8 BattleScript_PledgeCombined[];
extern const u8 BattleScript_PledgeReady[];
extern const u8 BattleScript_PrintCustomString[];

extern const u8 BattleScript_NoHealTargetAfterHealBlock[];
extern const u8 BattleScript_FaintAttacker[];
extern const u8 BattleScript_FaintRaidAttacker[];
extern const u8 BattleScript_FaintTarget[];
extern const u8 BattleScript_FaintRaidTarget[];
extern const u8 BattleScript_FaintScriptingBank[];
extern const u8 BattleScript_FaintRaidScriptingBank[];
extern const u8 BattleScript_BenjaminButterfreeDevolution[];
extern const u8 BattleScript_ButItFailedAttackstring[];
extern const u8 BattleScript_EnduredMsg[];
extern const u8 BattleScript_SideStatusWoreOffRet[];
extern const u8 RecoverBS[];
extern const u8 BattleScript_LifeDewFail[];
extern const u8 BattleScript_CouldntFullyProtect[];
extern const u8 BattleScript_TeleportSwitch[];
extern const u8 BattleScript_TeleportFlee[];

extern const u8 BattleScript_TargetSleepHeal[];
extern const u8 BattleScript_TargetBurnHeal[];
extern const u8 BattleScript_ItemSteal[];
extern const u8 BattleScript_KnockedOff[];
extern const u8 BattleScript_AbilityWasSuppressed[];
extern const u8 BattleScript_StatUpPartner[];
extern const u8 BattleScript_EatEffectBankBerry[];
extern const u8 BattleScript_SetTerrain[];
extern const u8 BattleScript_MaxMoveRaiseStatTeam[];
extern const u8 BattleScript_MaxMoveLowerStatFoes[];
extern const u8 BattleScript_MaxMoveSetWeather[];
extern const u8 BattleScript_MaxMoveSetTerrain[];
extern const u8 BattleScript_MaxMoveEffectSporeFoes[];
extern const u8 BattleScript_MaxMoveParalyzeFoes[];
extern const u8 BattleScript_MaxMovePoisonFoes[];
extern const u8 BattleScript_MaxMovePayDayConfuseFoes[];
extern const u8 BattleScript_MaxMoveCritUp[];
extern const u8 BattleScript_MaxMoveTrap[];
extern const u8 BattleScript_MaxMoveInfatuation[];
extern const u8 BattleScript_MaxMoveRecycle[];
extern const u8 BattleScript_MaxMoveHealTeam[];
extern const u8 BattleScript_MaxMoveSetStealthRock[];
extern const u8 BattleScript_MaxMoveDefog[];
extern const u8 BattleScript_MaxMovePoisonParalyzeFoes[];
extern const u8 BattleScript_MaxMoveSpite[];
extern const u8 BattleScript_MaxMoveSetGravity[];
extern const u8 BattleScript_MaxMoveSetYawn[];
extern const u8 BattleScript_MaxMoveTrapAndDamageFoes[];
extern const u8 BattleScript_MaxMoveLowerEvasivenessFoes[];
extern const u8 BattleScript_MaxMoveAromatherapy[];
extern const u8 BattleScript_MaxMoveConfuseFoes[];
extern const u8 BattleScript_MaxMoveTormentFoes[];
extern const u8 BattleScript_MaxMoveLowerSpeed2Foes[];

//Strings
extern const u8 gText_SuperEffectiveTarget[];
extern const u8 gText_SuperEffectiveTwoFoes[];
extern const u8 gText_NotVeryEffectiveTarget[];
extern const u8 gText_NotVeryEffectiveTwoFoes[];
extern const u8 gText_DoesntAffectTwoFoes[];
extern const u8 gText_CriticalHitTarget[];
extern const u8 gText_CriticalHitTwoFoes[];
extern const u8 CraftyShieldProtectedString[];
extern const u8 MatBlockProtectedString[];
extern const u8 QuickGuardProtectedString[];
extern const u8 WideGuardProtectedString[];
extern const u8 gText_RaidShieldProtected[];
extern const u8 PowerTrickString[];
extern const u8 PowerSwapString[];
extern const u8 GuardSwapString[];
extern const u8 SpeedSwapString[];
extern const u8 HeartSwapString[];
extern const u8 PowerSplitString[];
extern const u8 GuardSplitString[];
extern const u8 SwampString[];
extern const u8 SeaOfFireString[];
extern const u8 RainbowString[];
extern const u8 TrickRoomEndString[];
extern const u8 WonderRoomEndString[];
extern const u8 MagicRoomEndString[];
extern const u8 GravityEndString[];
extern const u8 TrickRoomSetString[];
extern const u8 WonderRoomSetString[];
extern const u8 MagicRoomSetString[];
extern const u8 GravitySetString[];
extern const u8 IonDelugeShowerString[];
extern const u8 NoMoreAirborneFromGravityString[];
extern const u8 Bank0AWasBroughtDownString[];
extern const u8 TargetTransformedIntoType[];
extern const u8 ThirdTypeAddedString[];
extern const u8 FairyLockString[];
extern const u8 HappyHourString[];
extern const u8 CelebrateString[];
extern const u8 HoldHandsString[];
extern const u8 TailwindSetString[];
extern const u8 LuckyChantSetString[];
extern const u8 WorrySeedString[];
extern const u8 AbilitySuppressedString[];
extern const u8 EntrainmentString[];
extern const u8 SimpleBeamString[];
extern const u8 gText_SetAuroraVeil[];
extern const u8 FreedFromSkyDropString[];
extern const u8 StringEnduredHitWithSturdy[];
extern const u8 PrimalRainEndString[];
extern const u8 PrimalSunEndString[];
extern const u8 PrimalAirCurrentEndString[];
extern const u8 RemovedEntryHazardsString[];
extern const u8 RemovedEntryHazardsTargetSideString[];
extern const u8 SpikesLayString[];
extern const u8 StealthRockLayString[];
extern const u8 ToxicSpikesLayString[];
extern const u8 StickyWebLayString[];
extern const u8 gText_SteelsurgeLay[];
extern const u8 CraftyShieldSetString[];
extern const u8 MatBlockSetString[];
extern const u8 QuickGuardSetString[];
extern const u8 WideGuardSetString[];
extern const u8 gText_ScreenRaisedStat[];
extern const u8 RemovedEntryHazardsString[];
extern const u8 SpikesLayString[];
extern const u8 StealthRockLayString[];
extern const u8 ToxicSpikesLayString[];
extern const u8 StickyWebLayString[];

extern const u8 ElectricTerrainSetString[];
extern const u8 GrassyTerrainSetString[];
extern const u8 MistyTerrainSetString[];
extern const u8 PsychicTerrainSetString[];

#define gText_TargetAlreadyAsleep (u8*) 0x83FB57C
#define gText_TargetAlreadyConfused (u8*) 0x83FB885
extern const u8 gText_TeamProtectedBySafeguard[];
extern const u8 gText_TargetAlreadyHasStatusCondition[];
extern const u8 gText_AlreadyDrowsy[];
extern const u8 gText_TargetWrappedInMistyTerrain[];
extern const u8 gText_TargetWrappedInElectricTerrain[];
extern const u8 gText_FlowerVeilProtects[];
extern const u8 gText_SweetVeilProtects[];
extern const u8 gText_SleepClausePrevents[];
#define gText_CantFallAsleepDuringUproar (u8*) 0x83FBDC4

#define gText_TargetAlreadyParalyzed (u8*) 0x83FB78A
#define gText_TargetAlreadyBurned (u8*) 0x83FB67B
#define gText_TargetAlreadyPoisoned (u8*) 0x83FB5F8

extern const u8 IonDelugeShowerString[];
extern const u8 TargetStatsResetString[];
extern const u8 TerrainEndString[];
extern const u8 RoastedBerryString[];
extern const u8 gText_TargetWasInfested[];
extern const u8 gText_TargetWasCaughtInSnapTrap[];

extern const u8 BattleScript_HealingWishHeal[];
extern const u8 BattleScript_LunarDanceHeal[];
extern const u8 BattleScript_EntryHazardsHurtReturn[];
extern const u8 BattleScript_SpikesHurt[];
extern const u8 BattleScript_SRHurt[];
extern const u8 BattleScript_SteelsurgeHurt[];
extern const u8 BattleScript_TSPoison[];
extern const u8 BattleScript_TSHarshPoison[];
extern const u8 BattleScript_TSAbsorb[];
extern const u8 BattleScript_StickyWebSpeedDrop[];
extern const u8 BattleScript_SuccessForceOut[];
extern const u8 BattleScript_TrainerSlideMsgRet[];

extern const u8 gText_SurroundedByGMaxVineLash[];
extern const u8 gText_SurroundedByGMaxWildfire[];
extern const u8 gText_SurroundedByGMaxCannonade[];
extern const u8 gText_SurroundedByGMaxVolcalith[];
