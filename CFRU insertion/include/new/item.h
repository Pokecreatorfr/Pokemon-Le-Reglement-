#pragma once

#include "../global.h"
#include "../../src/config.h"

/**
 * \file item.h
 * \brief Contains functions relating to items and the bag.
 *		  Also helps with TM/HM expansion and the bag expansion.
 */

//Exported Functions
u16 SanitizeItemId(u16 itemId);
const u8* ItemId_GetName(u16 itemId);
u8 ItemId_GetHoldEffect(u16 itemId);
u8 ItemId_GetHoldEffectParam(u16 itemId);
u8 ItemId_GetMystery2Id(u16 itemId);
bool8 IsMegaStone(u16 item);
bool8 IsPrimalOrb(u16 item);
bool8 IsZCrystal(u16 item);
bool8 IsTypeZCrystal(u16 item, u8 moveType);
bool8 IsBerry(u16 item);
bool8 IsPinchBerryItemEffect(u8 itemEffect);
bool8 IsGem(u16 item);
bool8 IsTMHM(u16 item);
u8 TMIdFromItemId(u16 itemId);
u8 BerryIdFromItemId(u16 item);
bool8 GetSetItemObtained(u16 item, u8 caseId);
u8 ReformatItemDescription(u16 item, u8* dest, u8 maxChars);

u32 CanMonLearnTMHM(struct Pokemon* mon, u8 tm);
bool8 CanMonLearnTutorMove(struct Pokemon* mon, u8 tutorId);
u16 GetExpandedTutorMove(u8 tutorId);
void CopyTMName(u8* dst, u16 itemId);
u8 CanMonLearnTMTutor(struct Pokemon* mon, u16 item, u8 tutor);

bool8 IsItemRegistered(u16 item);
bool8 CanRegisterNewItem(void);
void RegisterItem(u16 item);
void RemoveRegisteredItem(u16 item);
void CompactRegisteredItems(void);

u16 GetNumItemsInPocket(u8 pocket);
u16 GetCurrentPocketItemAmount(void);
bool8 DoesBagHaveBerry(void);
void SortBerriesOrTMHMs(struct BagPocket* bagPocket);
void SortItemsInBag(u8 pocket, u8 type);

//Hooked in Functions
u8 TryHandleExcuseForDracoMeteorTutor(struct Pokemon* mon);
u8 TryHandleExcuseForDracoMeteorTutorAlreadyKnow(void);
void CancelPartyMenuLearnTutor(u8 taskId);
u16 ItemIdToBattleMoveId(u16 item);
void LoadTMNameWithNo(u8* dst, u16 itemId);
void LoadTmHmNameInMart(u16 item);
void* LoadTmHmMartDescription(u16 item);
bool8 CheckIsHmMove(u16 move);
bool8 CheckTmHmInFront(u16 item);
u8 CheckDiscIsTmHm(struct Sprite* disc, u16 itemId);
u8 TmHMDiscPosition(unusedArg struct Sprite* disc, u8 tmId);
bool8 CheckReusableTMs(u16 item);
u8 CheckHmSymbol(u16 item);
bool8 CheckSellTmHm(u16 item);
void CheckTmPurchase(u16 item, u8 taskId);
bool8 CheckBuyableTm(u16 item, u8 taskId);
u16 CheckTmPrice(u16 item);
u8 CheckSingleBagTm(u16 item);
const void* FixTmHmDiscPalette(u8 type);

void CompactItemsInBagPocket(struct ItemSlot* itemSlots, u16 amount);
void Task_ReturnToItemListAfterItemPurchase(u8 taskId);
void SetMemoryForBagStorage(void);
void AllocateBagItemListBuffers(void);
bool8 AllocateBerryPouchListBuffers(void);
void PokeDudeBackupBag(void);
void PokeDudeRestoreBag(void);
void PokeDudeBackupKeyItemsTMs(void);
void PokeDudeRestoreKeyItemsTMs(void);
void StoreBagItemCount(void);
bool8 TrySetupSortBag(u8 taskId);
void LoadBagSorterMenuOptions(void);
void PrintBagSortItemQuestion(u8 windowId);

//Exported Constants
enum
{
    CAN_LEARN_MOVE,
    CANNOT_LEARN_MOVE,
    ALREADY_KNOWS_MOVE,
    CANNOT_LEARN_MOVE_IS_EGG,
};

enum ItemObtainFlags
{
    FLAG_GET_OBTAINED,
    FLAG_SET_OBTAINED,
};

#define REGISTERED_ITEM_COUNT 6
#define NUM_TMSHMS NUM_TMS + NUM_HMS	// never change this

#define gTMHMMoves ((const u16*) *((u32*) 0x8125A8C))
#define gTutorMoves ((const u16*) *((u32*) 0x8120BE4))

enum ItemType //Sorted in this order
{
	ITEM_TYPE_FIELD_USE,
	ITEM_TYPE_HEALTH_RECOVERY,
	ITEM_TYPE_STATUS_RECOVERY,
	ITEM_TYPE_PP_RECOVERY,
	ITEM_TYPE_STAT_BOOST_DRINK,
	ITEM_TYPE_STAT_BOOST_WING,
	ITEM_TYPE_EVOLUTION_STONE,
	ITEM_TYPE_EVOLUTION_ITEM,
	ITEM_TYPE_BATTLE_ITEM,
	ITEM_TYPE_FLUTE,
	ITEM_TYPE_STAT_BOOST_HELD_ITEM,
	ITEM_TYPE_HELD_ITEM,
	ITEM_TYPE_GEM,
	ITEM_TYPE_PLATE,
	ITEM_TYPE_MEMORY,
	ITEM_TYPE_DRIVE,
	ITEM_TYPE_INCENSE,
	ITEM_TYPE_MEGA_STONE,
	ITEM_TYPE_Z_CRYSTAL,
	ITEM_TYPE_NECTAR,
	ITEM_TYPE_SELLABLE,
	ITEM_TYPE_RELIC,
	ITEM_TYPE_SHARD,
	ITEM_TYPE_FOSSIL,
	ITEM_TYPE_MAIL,
};
