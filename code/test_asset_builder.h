#if !defined(TEST_ASSET_BUILDER_H)

#include <stdio.h>
#include <stdlib.h>
#include "handmade_platform.h"
#include "handmade_asset_type_id.h"
#include "handmade_file_formats.h"

struct bitmap_id
{
    uint32 Value;
};

struct sound_id
{
    uint32 Value;
};

struct asset_bitmap_info
{
    char *FileName;
    r32 AlignPercentage[2];
};

struct asset_sound_info
{
    char *FileName;
    u32 FirstSampleIndex;
    u32 SampleCount;
    sound_id NextIDToPlay;
};

struct asset
{
    u64 DataOffset;
    u32 FirstTagIndex;
    u32 OnePastLastTagIndex;
    union {
        asset_bitmap_info Bitmap;
        asset_sound_info Sound;
    };
};

#define VERY_LARGE_NUMBER 4096

struct game_assets
{
    u32 TagCount;
    hha_tag Tags[VERY_LARGE_NUMBER];

    u32 AssetCount;
    asset Assets[VERY_LARGE_NUMBER];

    u32 AssetTypeCount;
    hha_asset_type AssetTypes[Asset_Count];

    hha_asset_type *DEBUGAssetType;
    asset *DEBUGAsset;
};

#define TEST_ASSET_BUILDER_H
#endif