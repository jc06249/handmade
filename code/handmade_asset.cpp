
#pragma pack(push, 1)
struct bitmap_header
{
    uint16 FileType;
    uint32 FileSize;
    uint16 Reserved1;
    uint16 Reserved2;
    uint32 BitmapOffset;
    uint32 Size;
    int32 Width;
    int32 Height;
    uint16 Planes;
    uint16 BitsPerPixel;
    uint32 Compression;
    uint32 SizeOfBitmap;
    int32 HorzResolution;
    int32 VertResolution;
    uint32 ColorsUsed;
    uint32 ColorsImportant;

    uint32 RedMask;
    uint32 GreenMask;
    uint32 BlueMask;
};
#pragma pack(pop)

inline v2 TopDownAlign(loaded_bitmap *Bitmap, v2 Align)
{
    Align.y = (real32)(Bitmap->Height - 1) - Align.y;

    Align.x = SafeRatio0(Align.x, (real32)Bitmap->Width);
    Align.y = SafeRatio0(Align.y, (real32)Bitmap->Height);

    return(Align);
}

internal loaded_bitmap DEBUGLoadBMP(char *FileName, v2 AlignPercentage = V2(0.5f, 0.5f))
{
    loaded_bitmap Result = {};

    debug_read_file_result ReadResult = DEBUGPlatformReadEntireFile(FileName);
    if(ReadResult.ContentsSize != 0)
    {
        bitmap_header *Header = (bitmap_header *)ReadResult.Contents;
        uint32 *Pixels = (uint32 *)((uint8 *)ReadResult.Contents + Header->BitmapOffset);
        Result.Memory = Pixels;
        Result.Width = Header->Width;
        Result.Height = Header->Height;
        Result.AlignPercentage = AlignPercentage;
        Result.WidthOverHeight = SafeRatio0((real32)Result.Width, (real32)Result.Height);

        Assert(Header->Height >= 0);
        Assert(Header->Compression == 3);

        // NOTE: If you are using this generically for some reason,
        // please remember that BMP files CAN GO IN EITHER DIRECTION and
        // the height will be negative for top-down.
        // (Also, there can be compression, etc., etc... DON'T think this
        // is complete BMP loading code because it isn't!!)

        // NOTE: Byte order in memory is determined by the Header itself,
        // so we have to read out the masks and convert the pixels ourselves.
        uint32 RedMask = Header->RedMask;
        uint32 GreenMask = Header->GreenMask;
        uint32 BlueMask = Header->BlueMask;
        uint32 AlphaMask = ~(RedMask | GreenMask | BlueMask);

        bit_scan_result RedScan = FindLeastSignificantSetBit(RedMask);
        bit_scan_result GreenScan = FindLeastSignificantSetBit(GreenMask);
        bit_scan_result BlueScan = FindLeastSignificantSetBit(BlueMask);
        bit_scan_result AlphaScan = FindLeastSignificantSetBit(AlphaMask);

        Assert(RedScan.Found);
        Assert(GreenScan.Found);
        Assert(BlueScan.Found);
        Assert(AlphaScan.Found);

        int32 RedshiftDown = (int32)RedScan.Index;
        int32 GreenshiftDown = (int32)GreenScan.Index;
        int32 BlueshiftDown = (int32)BlueScan.Index;
        int32 AlphashiftDown = (int32)AlphaScan.Index;

        uint32 *SourceDest = Pixels;
        for(int32 Y = 0; Y < Header->Height; ++Y)
        {
            for(int32 X = 0; X < Header->Width; ++X)
            {
                uint32 C = *SourceDest;

                v4 Texel = {(real32)((C & RedMask) >> RedshiftDown),
                            (real32)((C & GreenMask) >> GreenshiftDown),
                            (real32)((C & BlueMask) >> BlueshiftDown),
                            (real32)((C & AlphaMask) >> AlphashiftDown)};

                Texel = SRGB255ToLinear1(Texel);
#if 1
                Texel.rgb *= Texel.a;
#endif
                Texel = Linear1ToSRGB255(Texel);

                *SourceDest++ = (((uint32)(Texel.a + 0.5f) << 24) |
                                 ((uint32)(Texel.r + 0.5f) << 16) |
                                 ((uint32)(Texel.g + 0.5f) << 8) |
                                 ((uint32)(Texel.b + 0.5f) << 0));
            }
        }
    }

    Result.Pitch = Result.Width * BITMAP_BYTES_PER_PIXEL;

#if 0
    Result.Memory = (uint8 *)Result.Memory + Result.Pitch * (Result.Height - 1);
    Result.Pitch = -Result.Pitch;
#endif

    return(Result);
}

struct WAVE_header
{
    uint32 RIFFID;
    uint32 Size;
    uint32 WAVEID;
};

#define RIFF_CODE(a, b, c, d) (((uint32)a << 0) | ((uint32)(b) << 8) | ((uint32)(c) << 16) | ((uint32)(d) << 24))
enum
{
    WAVE_ChunkID_fmt = RIFF_CODE('f', 'm', 't', ' '),
    WAVE_ChunkID_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
    WAVE_ChunkID_WAVE = RIFF_CODE('W', 'A', 'V', 'E'),
};

struct WAVE_chunk
{
    uint32 ID;
    uint32 Size;
};

struct WAVE_fmt
{
    uint16 wFormatTag;
    uint16 nChannels;
    uint32 nSamplesPerSec;
    uint32 nAvgBytesPerSec;
    uint32 nBlockAlign;
    uint16 wBitsPerSample;
    uint16 CbSize;
    uint16 wValidBitsPerSample;
    uint32 dwChannelMask;
    uint8 SubFormat[16];
};

internal loaded_sound DEBUGLoadWAV(char *FileName)
{
    loaded_sound Result = {};

    debug_read_file_result ReadResult = DEBUGPlatformReadEntireFile(FileName);
    if(ReadResult.ContentsSize != 0)
    {
        WAVE_header *Header = (WAVE_header *)ReadResult.Contents;
        Assert(Header->RIFFID == WAVE_ChunkID_RIFF);
        Assert(Header->WAVEID == WAVE_ChunkID_WAVE);
    }

    return(Result);
}

struct load_bitmap_work
{
    game_assets *Assets;
    bitmap_id ID;
    task_with_memory *Task;
    loaded_bitmap *Bitmap;

    asset_state FinalState;
};
internal PLATFORM_WORK_QUEUE_CALLBACK(LoadBitmapWork)
{
    load_bitmap_work *Work = (load_bitmap_work *)Data;

    asset_bitmap_info *Info = Work->Assets->BitmapInfos + Work->ID.Value;
    *Work->Bitmap = DEBUGLoadBMP(Info->FileName, Info->AlignPercentage);

    CompletePreviousWritesBeforeFutureWrites;

    Work->Assets->Bitmaps[Work->ID.Value].Bitmap = Work->Bitmap;
    Work->Assets->Bitmaps[Work->ID.Value].State = Work->FinalState;

    EndTaskWithMemory(Work->Task);
}

internal void LoadBitmap(game_assets *Assets, bitmap_id ID)
{
    if(ID.Value &&
       (AtomicCompareExchangeUInt32((uint32 *)&Assets->Bitmaps[ID.Value].State, AssetState_Unloaded, AssetState_Queued) == AssetState_Unloaded))
    {
        task_with_memory *Task = BeginTaskWithMemory(Assets->TranState);
        if(Task)
        {
            load_bitmap_work *Work = PushStruct(&Task->Arena, load_bitmap_work);

            Work->Assets = Assets;
            Work->ID = ID;
            Work->Task = Task;
            Work->Bitmap = PushStruct(&Assets->Arena, loaded_bitmap);
            Work->FinalState = AssetState_Loaded;

            PlatformAddEntry(Assets->TranState->LowPriorityQueue, LoadBitmapWork, Work);
        }
    }
}

struct load_sound_work
{
    game_assets *Assets;
    sound_id ID;
    task_with_memory *Task;
    loaded_sound *Sound;

    asset_state FinalState;
};

internal PLATFORM_WORK_QUEUE_CALLBACK(LoadSoundWork)
{
    load_sound_work *Work = (load_sound_work *)Data;

    asset_sound_info *Info = Work->Assets->SoundInfos + Work->ID.Value;
    *Work->Sound = DEBUGLoadWAV(Info->FileName);

    CompletePreviousWritesBeforeFutureWrites;

    Work->Assets->Sounds[Work->ID.Value].Sound = Work->Sound;
    Work->Assets->Bitmaps[Work->ID.Value].State = Work->FinalState;

    EndTaskWithMemory(Work->Task);
}

internal void LoadSound(game_assets *Assets, sound_id ID)
{
    if(ID.Value &&
       (AtomicCompareExchangeUInt32((uint32 *)&Assets->Sounds[ID.Value].State, AssetState_Unloaded, AssetState_Queued) == AssetState_Unloaded))
    {
        task_with_memory *Task = BeginTaskWithMemory(Assets->TranState);
        if(Task)
        {
            load_sound_work *Work = PushStruct(&Task->Arena, load_sound_work);

            Work->Assets = Assets;
            Work->ID = ID;
            Work->Task = Task;
            Work->Sound = PushStruct(&Assets->Arena, loaded_sound);
            Work->FinalState = AssetState_Loaded;

            PlatformAddEntry(Assets->TranState->LowPriorityQueue, LoadSoundWork, Work);
        }
    }
}

internal bitmap_id BestMatchAsset(game_assets *Assets, asset_type_id TypeID,
                                  asset_vector *MatchVector, asset_vector *WeightVector)
{
    bitmap_id Result = {};

    real32 BestDiff = Real32Maximum;
    asset_type *Type = Assets->AssetTypes + TypeID;
    for(uint32 AssetIndex = Type->FirstAssetIndex; AssetIndex < Type->OnePastLastAssetIndex; ++AssetIndex)
    {
        asset *Asset = Assets->Assets + AssetIndex;

        real32 TotalWeightedDiff = 0.0f;
        for (uint32 TagIndex = Asset->FirstTagIndex; TagIndex < Asset->OnePastLastTagIndex; ++TagIndex)
        {
            asset_tag *Tag = Assets->Tags + TagIndex;

            real32 A = MatchVector->E[Tag->ID];
            real32 B = Tag->Value;
            real32 D0 = AbsoluteValue(A - B);
            real32 D1 = AbsoluteValue((A - Assets->TagRange[Tag->ID] * SignOf(A)) - B);
            real32 Difference = Minimum(D0, D1);

            real32 Weighted = WeightVector->E[Tag->ID] * Difference;
            TotalWeightedDiff += Weighted;
        }
        if(BestDiff > TotalWeightedDiff)
        {
            BestDiff = TotalWeightedDiff;
            Result.Value = Asset->SlotID;
        }
    }

    return(Result);
}

internal bitmap_id RandomAssetFrom(game_assets *Assets, asset_type_id TypeID, random_series *Series)
{
    bitmap_id Result = {};

    asset_type *Type = Assets->AssetTypes + TypeID;
    if(Type->FirstAssetIndex != Type->OnePastLastAssetIndex)
    {
        uint32 Count = (Type->OnePastLastAssetIndex - Type->FirstAssetIndex);
        uint32 Choice = RandomChoice(Series, Count);

        asset *Asset = Assets->Assets + Type->FirstAssetIndex + Choice;
        Result.Value = Asset->SlotID;
    }

    return(Result);
}

internal bitmap_id GetFirstBitmapID(game_assets *Assets, asset_type_id TypeID)
{
    bitmap_id Result = {};

    asset_type *Type = Assets->AssetTypes + TypeID;
    if(Type->FirstAssetIndex != Type->OnePastLastAssetIndex)
    {
        asset *Asset = Assets->Assets + Type->FirstAssetIndex;
        Result.Value = Asset->SlotID;
    }

    return(Result);
}

internal bitmap_id DEBUGAddBitmapInfo(game_assets *Assets, char *FileName, v2 AlignPercentage)
{
    Assert(Assets->DEBUGUsedBitmapCount < Assets->BitmapCount);
    bitmap_id ID = {Assets->DEBUGUsedBitmapCount++};

    asset_bitmap_info *Info = Assets->BitmapInfos + ID.Value;
    Info->FileName = FileName;
    Info->AlignPercentage = AlignPercentage;

    return(ID);
}

internal void BeginAssetType(game_assets *Assets, asset_type_id TypeID)
{
    Assert(Assets->DEBUGAssetType == 0);
    Assets->DEBUGAssetType = Assets->AssetTypes + TypeID;
    Assets->DEBUGAssetType->FirstAssetIndex = Assets->DEBUGUsedAssetCount;
    Assets->DEBUGAssetType->OnePastLastAssetIndex = Assets->DEBUGAssetType->FirstAssetIndex;
}

internal void AddBitmapAsset(game_assets *Assets, char *FileName, v2 AlignPercentage = V2(0.5f, 0.5f))
{
    Assert(Assets->DEBUGAssetType);
    Assert(Assets->DEBUGAssetType->OnePastLastAssetIndex < Assets->AssetCount);

    asset *Asset = Assets->Assets + Assets->DEBUGAssetType->OnePastLastAssetIndex++;
    Asset->FirstTagIndex = Assets->DEBUGUsedTagCount;
    Asset->OnePastLastTagIndex = Asset->FirstTagIndex;
    Asset->SlotID = DEBUGAddBitmapInfo(Assets, FileName, AlignPercentage).Value;

    Assets->DEBUGAsset = Asset;
}

internal void AddTag(game_assets *Assets, asset_tag_id ID, real32 Value)
{
    Assert(Assets->DEBUGAsset);

    ++Assets->DEBUGAsset->OnePastLastTagIndex;
    asset_tag *Tag = Assets->Tags + Assets->DEBUGUsedTagCount++;

    Tag->ID = ID;
    Tag->Value = Value;
}

internal void EndAssetType(game_assets *Assets)
{
    Assert(Assets->DEBUGAssetType);
    Assets->DEBUGUsedAssetCount = Assets->DEBUGAssetType->OnePastLastAssetIndex;
    Assets->DEBUGAssetType = 0;
    Assets->DEBUGAsset = 0;
}

internal game_assets * AllocateGameAssets(memory_arena *Arena, memory_index Size, transient_state *TranState)
{
    game_assets *Assets = PushStruct(Arena, game_assets);
    SubArena(&Assets->Arena, Arena, Size);
    Assets->TranState = TranState;

    for(uint32 TagType = 0; TagType < Tag_Count; ++TagType)
    {
        Assets->TagRange[TagType];
    }
    Assets->TagRange[Tag_FacingDirection] = Tau32;

    Assets->BitmapCount = 256 * Asset_Count;
    Assets->BitmapInfos = PushArray(Arena, Assets->BitmapCount, asset_bitmap_info);
    Assets->Bitmaps = PushArray(Arena, Assets->BitmapCount, asset_slot);

    Assets->SoundCount = 1;
    Assets->Sounds = PushArray(Arena, Assets->SoundCount, asset_slot);

    Assets->AssetCount = Assets->SoundCount + Assets->BitmapCount;
    Assets->Assets = PushArray(Arena, Assets->AssetCount, asset);

    Assets->TagCount = 1024*Asset_Count;
    Assets->Tags = PushArray(Arena, Assets->TagCount, asset_tag);

    Assets->DEBUGUsedBitmapCount = 1;
    Assets->DEBUGUsedAssetCount = 1;

    BeginAssetType(Assets, Asset_Shadow);
    AddBitmapAsset(Assets, "../data/test/test_hero_shadow.bmp", V2(0.5f, 0.156682029f));
    EndAssetType(Assets);

    BeginAssetType(Assets, Asset_Tree);
    AddBitmapAsset(Assets, "../data/test2/tree00.bmp", V2(0.493827164f, 0.295652181f));
    EndAssetType(Assets);

    BeginAssetType(Assets, Asset_Rock);
    AddBitmapAsset(Assets, "../data/test2/rock03.bmp", V2(0.5f, 0.65625f));
    EndAssetType(Assets);

    BeginAssetType(Assets, Asset_Grass);
    AddBitmapAsset(Assets, "../data/test2/grass00.bmp");
    AddBitmapAsset(Assets, "../data/test2/grass01.bmp");
    EndAssetType(Assets);

    BeginAssetType(Assets, Asset_Tuft);
    AddBitmapAsset(Assets,"../data/test2/tuft00.bmp");
    AddBitmapAsset(Assets,"../data/test2/tuft01.bmp");
    AddBitmapAsset(Assets,"../data/test2/tuft02.bmp");
    EndAssetType(Assets);


    BeginAssetType(Assets, Asset_Stone);
    AddBitmapAsset(Assets, "../data/test2/ground00.bmp");
    AddBitmapAsset(Assets, "../data/test2/ground01.bmp");
    AddBitmapAsset(Assets, "../data/test2/ground02.bmp");
    AddBitmapAsset(Assets, "../data/test2/ground03.bmp");
    EndAssetType(Assets);

    real32 AngleRight = 0.0f * Tau32;
    real32 AngleBack = 0.25f * Tau32;
    real32 AngleLeft = 0.5f * Tau32;
    real32 AngleFront = 0.75f * Tau32;

    v2 HeroAlign = {0.5f, 0.156682029f};

    BeginAssetType(Assets, Asset_Head);
    AddBitmapAsset(Assets, "../data/test/test_hero_right_head.bmp", HeroAlign);
    AddTag(Assets, Tag_FacingDirection, AngleRight);
    AddBitmapAsset(Assets, "../data/test/test_hero_back_head.bmp", HeroAlign);
    AddTag(Assets, Tag_FacingDirection, AngleBack);
    AddBitmapAsset(Assets, "../data/test/test_hero_left_head.bmp", HeroAlign);
    AddTag(Assets, Tag_FacingDirection, AngleLeft);
    AddBitmapAsset(Assets, "../data/test/test_hero_front_head.bmp", HeroAlign);
    AddTag(Assets, Tag_FacingDirection, AngleFront);
    EndAssetType(Assets);

    BeginAssetType(Assets, Asset_Cape);
    AddBitmapAsset(Assets, "../data/test/test_hero_right_cape.bmp", HeroAlign);
    AddTag(Assets, Tag_FacingDirection, AngleRight);
    AddBitmapAsset(Assets, "../data/test/test_hero_back_cape.bmp", HeroAlign);
    AddTag(Assets, Tag_FacingDirection, AngleBack);
    AddBitmapAsset(Assets, "../data/test/test_hero_left_cape.bmp", HeroAlign);
    AddTag(Assets, Tag_FacingDirection, AngleLeft);
    AddBitmapAsset(Assets, "../data/test/test_hero_front_cape.bmp", HeroAlign);
    AddTag(Assets, Tag_FacingDirection, AngleFront);
    EndAssetType(Assets);

    BeginAssetType(Assets, Asset_Torso);
    AddBitmapAsset(Assets, "../data/test/test_hero_right_torso.bmp", HeroAlign);
    AddTag(Assets, Tag_FacingDirection, AngleRight);
    AddBitmapAsset(Assets, "../data/test/test_hero_back_torso.bmp", HeroAlign);
    AddTag(Assets, Tag_FacingDirection, AngleBack);
    AddBitmapAsset(Assets, "../data/test/test_hero_left_torso.bmp", HeroAlign);
    AddTag(Assets, Tag_FacingDirection, AngleLeft);
    AddBitmapAsset(Assets, "../data/test/test_hero_front_torso.bmp", HeroAlign);
    AddTag(Assets, Tag_FacingDirection, AngleFront);
    EndAssetType(Assets);

    return(Assets);
}