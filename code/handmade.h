#if !defined(HANDMADE_H)

/*
    HANDMADE_INTERNAL:
    0 - Build for public release
    1 - Buiild for developer only

    HANDMADE_SLOW:
    0 - Not slow code allowed!
    1 - Slow code welcome.
*/

#include "handmade_platform.h"

#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.14159265359f

#if HANDMADE_SLOW
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

inline uint32 SafeTruncateUInt64(uint64 Value){
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return(Result);
}


inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex){
    Assert(ControllerIndex < ArrayCount(Input->Controllers));
    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return(Result);
}

struct canonical_position
{
    // TODO: Take the tile map X and Y and the tile X and Y.
    //       and pack them into the single 32-bit values for X and Y
    //       where there is some low bits for the tile index and
    //       the high bits are the file "page"

    int32 TileMapX;
    int32 TileMapY;

    int32 TileX;
    int32 TileY;

    // TODO: Convert there to math-friendly, resolution respresentation of
    //       world units relative to a tile.
    float TileRelX;
    float TileRelY;
};

// TODO: Is this ever necessary?
struct raw_position
{
    int32 TileMapX;
    int32 TileMapY;

    float X;
    float Y;
};

struct tile_map
{
    uint32 *Tiles;
};

struct world
{

    float TileSideInMeters;
    int32 TileSideInPixels;

    int32 CountX;
    int32 CountY;

    float UpperLeftX;
    float UpperLeftY;

    int32 TileMapCountX;
    int32 TileMapCountY;

    tile_map *TileMaps;
};

struct game_state
{
    //TODO: Player state should be canonical position now?
    int32 PlayerTileMapX;
    int32 PlayerTileMapY;

    float PlayerX;
    float PlayerY;
};

#define HANDMADE_H
#endif