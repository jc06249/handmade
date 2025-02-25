#if !defined(HANDMADE_ASSET_H)

struct loaded_sound
{
    // TODO: This could be shrunk to 12 bytes if the loaded_bitmap
    // ever got down that small!
    int16 *Samples[2];
    u32 SampleCount; // NOTE: This is the sample count divided by 8
    u32 ChannelCount;
};

enum asset_state
{
    AssetState_Unloaded,
    AssetState_Queued,
    AssetState_Loaded,
    AssetState_StateMask = 0xFFF,

    AssetState_Lock = 0x10000,
};

struct asset_memory_header
{
    asset_memory_header *Next;
    asset_memory_header *Prev;

    u32 AssetIndex;
    u32 TotalSize;
    union
    {
        loaded_bitmap Bitmap;
        loaded_sound Sound;
    };
};

struct asset
{
    u32 State;
    asset_memory_header *Header;

    hha_asset HHA;
    u32 FileIndex;
};

struct asset_bitmap_info
{
    char *FileName;
    v2 AlignPercentage;
};

struct asset_sound_info
{
    char *FileName;
    uint32 FirstSampleIndex;
    uint32 SampleCount;
    sound_id NextIDToPlay;
};

struct asset_vector
{
    real32 E[Tag_Count];
};

struct asset_type
{
    uint32 FirstAssetIndex;
    uint32 OnePastLastAssetIndex;
};

struct asset_file
{
    platform_file_handle Handle;

    // TODO: If we ever do thread stacks,
    // AssetTypeArray doesn't need to be kept hereprobably.
    hha_header Header;
    hha_asset_type *AssetTypeArray;

    u32 TagBase;
};

enum asset_memory_block_flags
{
    AssetMemory_Used = 0x1,
};
struct asset_memory_block
{
    asset_memory_block *Prev;
    asset_memory_block *Next;
    u64 Flags;
    memory_index Size;
};

struct game_assets
{
    // TODO: Not thrilled about this back-pointer
    struct transient_state *TranState;

    asset_memory_block MemorySentinel;

    asset_memory_header LoadedAssetSentinel;

    real32 TagRange[Tag_Count];

    u32 FileCount;
    asset_file *Files;

    uint32 TagCount;
    hha_tag *Tags;

    uint32 AssetCount;
    asset *Assets;

    asset_type AssetTypes[Asset_Count];

#if 0
    u8 *HHAContents;
    // NOTE: Structured assets
    //hero_bitmaps HeroBitmaps[4];

    // TODO: These should go away once we actually load an asset pack file
    uint32 DEBUGUsedAssetCount;
    uint32 DEBUGUsedTagCount;
    asset_type *DEBUGAssetType;
    asset *DEBUGAsset;
#endif
};

inline b32 IsLocked(asset *Asset)
{
    b32 Result = (Asset->State & AssetState_Lock);
    return(Result);
}

inline u32 GetState(asset *Asset)
{
    u32 Result = Asset->State & AssetState_StateMask;
    return(Result);
}

internal void MoveHeaderToFront(game_assets *Assets, asset *Asset);
inline loaded_bitmap *GetBitmap(game_assets *Assets, bitmap_id ID, b32 MustBeLocked)
{
    Assert(ID.Value <= Assets->AssetCount);
    asset *Asset = Assets->Assets + ID.Value;

    loaded_bitmap *Result = 0;
    if(GetState(Asset) >= AssetState_Loaded)
    {
        Assert(!MustBeLocked || (IsLocked(Asset)));
        CompletePreviousReadsBeforeFutureReads;
        Result = &Asset->Header->Bitmap;
        MoveHeaderToFront(Assets, Asset);
    }

    return(Result);
}

inline loaded_sound *GetSound(game_assets *Assets, sound_id ID)
{
    Assert(ID.Value <= Assets->AssetCount);
    asset *Asset = Assets->Assets + ID.Value;

    loaded_sound *Result = 0;
    if(Asset->State >= AssetState_Loaded)
    {
        CompletePreviousReadsBeforeFutureReads;
        Result = &Asset->Header->Sound;
        MoveHeaderToFront(Assets, Asset);
    }

    return(Result);
}

inline hha_sound *GetSoundInfo(game_assets *Assets, sound_id ID)
{
    Assert(ID.Value <= Assets->AssetCount);
    hha_sound *Result = &Assets->Assets[ID.Value].HHA.Sound;

    return(Result);
}

inline bool32 IsValid(bitmap_id ID)
{
    bool32 Result = ID.Value;

    return(Result);
}

inline bool32 IsValid(sound_id ID)
{
    bool32 Result = ID.Value;

    return(Result);
}

internal void LoadBitmap(game_assets *Assets, bitmap_id ID, b32 Locked);
inline void PrefetchBitmap(game_assets *Assets, bitmap_id ID, b32 Locked) {LoadBitmap(Assets, ID, Locked);}
internal void LoadSound(game_assets *Assets, sound_id ID);
inline void PrefetchSound(game_assets *Assets, sound_id ID) {LoadSound(Assets, ID);}

inline sound_id GetNextSoundInChain(game_assets *Assets, sound_id ID)
{
    sound_id Result = {};

    hha_sound *Info = GetSoundInfo(Assets, ID);
    switch (Info->Chain)
    {
        case HHASoundChain_None:
        {
            //NOTE: Nothing to do.
        } break;

        case HHASoundChain_Loop:
        {
            Result = ID;
        } break;

        case HHASoundChain_Advance:
        {
            Result.Value = ID.Value + 1;
        } break;
        
        default:
        {
            InvalidCodePath;
        }break;
    } 

    return(Result);
}

#define HANDMADE_ASSET_H
#endif