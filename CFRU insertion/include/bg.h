#pragma once

#include "global.h"

struct BGCntrlBitfield // for the I/O registers
{
    volatile u16 priority:2;
    volatile u16 charBaseBlock:2;
    volatile u16 field_0_2:4;
    volatile u16 field_1_0:5;
    volatile u16 areaOverflowMode:1;
    volatile u16 screenSize:2;
};

enum
{
    BG_CTRL_ATTR_VISIBLE = 1,
    BG_CTRL_ATTR_CHARBASEINDEX = 2,
    BG_CTRL_ATTR_MAPBASEINDEX = 3,
    BG_CTRL_ATTR_SCREENSIZE = 4,
    BG_CTRL_ATTR_PALETTEMODE = 5,
    BG_CTRL_ATTR_PRIORITY = 6,
    BG_CTRL_ATTR_MOSAIC = 7,
    BG_CTRL_ATTR_WRAPAROUND = 8,
};

enum
{
	BG_ATTR_CHARBASEINDEX = 1,
	BG_ATTR_MAPBASEINDEX,
	BG_ATTR_SCREENSIZE,
	BG_ATTR_PALETTEMODE,
	BG_ATTR_MOSAIC,
	BG_ATTR_WRAPAROUND,
	BG_ATTR_PRIORITY,
	BG_ATTR_METRIC,
	BG_ATTR_TYPE,
	BG_ATTR_BASETILE,
};

enum BgTileAllocMode
{
    BG_TILE_FIND_FREE_SPACE,
    BG_TILE_ALLOC,
    BG_TILE_FREE,
};

struct BgTemplate
{
    u32 bg:2;                   // 0x1, 0x2 -> 0x3
    u32 charBaseIndex:2;        // 0x4, 0x8 -> 0xC
    u32 mapBaseIndex:5;         // 0x10, 0x20, 0x40, 0x80, 0x100 -> 0x1F0
    u32 screenSize:2;           // 0x200, 0x400 -> 0x600
    u32 paletteMode:1;          // 0x800
    u32 priority:2;             // 0x1000, 0x2000 > 0x3000
    u32 baseTile:10;
};

void __attribute__((long_call)) InitBgsFromTemplates(u8 layer, const struct BgTemplate* config, u8 layers);
u16 __attribute__((long_call)) LoadBgTiles(u8 bg, const void* src, u16 size, u16 destOffset);

void __attribute__((long_call)) ShowBg(u8 bg);
void __attribute__((long_call)) HideBg(u8 bg);
void __attribute__((long_call)) SetBgAttribute(u8 bg, u8 attributeId, u8 value);
u16 __attribute__((long_call)) GetBgAttribute(u8 bg, u8 attributeId);
u32 __attribute__((long_call)) ChangeBgX(u8 bg, u32 value, u8 op);
u32 __attribute__((long_call)) ChangeBgY(u8 bg, u32 value, u8 op);

void __attribute__((long_call)) CopyBgTilemapBufferToVram(u8 bg);
void __attribute__((long_call)) FillBgTilemapBufferRect(u8 bg, u16 tileNum, u8 x, u8 y, u8 width, u8 height, u8 palette);
void __attribute__((long_call)) SetBgTilemapBuffer(u8 bg, void *tilemap);
bool8 __attribute__((long_call)) IsDma3ManagerBusyWithBgCopy(void);
void __attribute__((long_call)) CopyToBgTilemapBuffer(u8 bg, const void *src, u16 mode, u16 destOffset);
void __attribute__((long_call)) FillBgTilemapBufferRect_Palette0(u8 bg, u16 tileNum, u8 x, u8 y, u8 width, u8 height);
void __attribute__((long_call)) ResetTempTileDataBuffers(void);
u16 __attribute__((long_call)) LoadBgTilemap(u8 bg, const void *src, u16 size, u16 destOffset);

/*
void ResetBgs(void);
u8 GetBgMode(void);
void ResetBgControlStructs(void);
void Unused_ResetBgControlStruct(u8 bg);
void SetBgControlAttributes(u8 bg, u8 charBaseIndex, u8 mapBaseIndex, u8 screenSize, u8 paletteMode, u8 priority, u8 mosaic, u8 wraparound);
u16 GetBgControlAttribute(u8 bg, u8 attributeId);
u8 LoadBgVram(u8 bg, const void *src, u16 size, u16 destOffset, u8 mode);
void SetTextModeAndHideBgs(void);
bool8 IsInvalidBg(u8 bg);
int BgTileAllocOp(int bg, int offset, int count, int mode);
//void ResetBgsAndClearDma3BusyFlags(bool32 enableWindowTileAutoAlloc);
void InitBgsFromTemplates(u8 bgMode, const struct BgTemplate *templates, u8 numTemplates);
void InitBgFromTemplate(const struct BgTemplate *template);
void SetBgMode(u8 bgMode);
u16 Unused_LoadBgPalette(u8 bg, const void *src, u16 size, u16 destOffset);
u32 GetBgX(u8 bg);
u32 ChangeBgY_ScreenOff(u8 bg, u32 value, u8 op);
u32 GetBgY(u8 bg);
void SetBgAffine(u8 bg, u32 srcCenterX, u32 srcCenterY, s16 dispCenterX, s16 dispCenterY, s16 scaleX, s16 scaleY, u16 rotationAngle);
u8 Unused_AdjustBgMosaic(u8 a1, u8 a2);
void UnsetBgTilemapBuffer(u8 bg);
void* GetBgTilemapBuffer(u8 bg);
void CopyToBgTilemapBufferRect(u8 bg, const void* src, u8 destX, u8 destY, u8 width, u8 height);
void CopyToBgTilemapBufferRect_ChangePalette(u8 bg, const void *src, u8 destX, u8 destY, u8 rectWidth, u8 rectHeight, u8 palette);
void CopyRectToBgTilemapBufferRect(u8 bg, const void* src, u8 srcX, u8 srcY, u8 srcWidth, u8 srcHeight, u8 destX, u8 destY, u8 rectWidth, u8 rectHeight, u8 palette1, u16 tileOffset, u16 palette2);
void WriteSequenceToBgTilemapBuffer(u8 bg, u16 firstTileNum, u8 x, u8 y, u8 width, u8 height, u8 paletteSlot, s16 tileNumDelta);
u16 GetBgMetricTextMode(u8 bg, u8 whichMetric);
u32 GetBgMetricAffineMode(u8 bg, u8 whichMetric);
u32 GetTileMapIndexFromCoords(s32 x, s32 y, s32 screenSize, u32 screenWidth, u32 screenHeight);
void CopyTileMapEntry(u16 *src, u16 *dest, s32 palette1, u32 tileOffset, u32 palette2);
u32 GetBgType(u8 bg);
bool32 IsInvalidBg32(u8 bg);
bool32 IsTileMapOutsideWram(u8 bg);

extern bool32 gWindowTileAutoAllocEnabled;
*/
