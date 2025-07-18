#if !defined(HANDMADE_FILE_FORMATS_H)

enum asset_tag_id
{
    Tag_Smoothness,
    Tag_Flatness,
    Tag_FacingDirection, // NOTE: Angle in radians off of due right
    Tag_UnicodeCodePoint,

    Tag_Count,
};

enum asset_type_id
{
    Asset_None,

    //
    // NOTE: Bitmaps!
    //

    Asset_Shadow,
    Asset_Tree,
    Asset_Sword,
    // Asset_Stairwell,
    Asset_Rock,

    Asset_Grass,
    Asset_Tuft,
    Asset_Stone,

    Asset_Head,
    Asset_Cape,
    Asset_Torso,

    Asset_Font,

    //
    // NOTE: Sounds!
    //

    Asset_Bloop,
    Asset_Crack,
    Asset_Drop,
    Asset_Glide,
    Asset_Music,
    Asset_Puhp,

    //
    //
    //

    Asset_Count,
};

#define HHA_CODE(a, b, c, d) (((uint32)a << 0) | ((uint32)(b) << 8) | ((uint32)(c) << 16) | ((uint32)(d) << 24))

#pragma pack(push, 1)
struct bitmap_id
{
    u32 Value;
};

struct sound_id
{
    u32 Value;
};

struct hha_header
{
#define HHA_MAGIC_VALUE HHA_CODE('h', 'h', 'a', 'f')
    u32 MagicValue;

#define HHA_VERSION 0
    u32 Version;

    u32 TagCount;
    u32 AssetTypeCount;
    u32 AssetCount;

    u64 Tags; //hha_tag[TagCount]
    u64 AssetTypes; // hha_asset_type[AssetTypeCount]
    u64 Assets; //hha_asset[AssetCount]

    // TODO: Primacy numbers for asset files?
};

struct hha_tag
{
    u32 ID;
    r32 Value;
};

struct hha_asset_type
{
    u32 TypeID;
    u32 FirstAssetIndex;
    u32 OnePastLastAssetIndex;
};

enum hha_sound_chain
{
    HHASoundChain_None,
    HHASoundChain_Loop,
    HHASoundChain_Advance,
};

struct hha_bitmap
{
    u32 Dim[2];
    r32 AlignPercentage[2];
};

struct hha_sound
{
    u32 SampleCount;
    u32 ChannelCount;
    u32 Chain; // NOTE: hha_sound_chain
};

struct hha_asset
{
    u64 DataOffset;
    u32 FirstTagIndex;
    u32 OnePastLastTagIndex;
    union 
    {
        hha_bitmap Bitmap;
        hha_sound Sound;
    };
};

#pragma pack(pop)

#define HANDMADE_FILE_FORMATS_H
#endif
