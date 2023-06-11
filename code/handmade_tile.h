
#if !defined(HANDMADE_TILE_H)

struct tile_map_position
{
    // NOTE: These are fixed point tile locations. The high
    // bits are tile chunk index, and the low bits are the tile
    // index in the chunk.
    uint32 AbsTileX;
    uint32 AbsTileY;

    // TODO: Should these be from the center of the tile?
    // TODO: Rename to offset X and Y
    real32 TileRelX;
    real32 TileRelY;
};

struct tile_chunk_position
{
    uint32 TileChunkX;
    uint32 TileChunkY;

    uint32 RelTileX;
    uint32 RelTileY;
};

struct tile_chunk
{
    uint32 *Tiles;
};

struct tile_map
{
    uint32 ChunkShift;
    uint32 ChunkMask;
    uint32 ChunkDim;

    real32 TileSideInMeters;
    int32 TileSideInPixels;
    real32 MetersToPixels;

    // TODO: Beginner's sparsness
    uint32 TileChunkCountX;
    uint32 TileChunkCountY;

    tile_chunk *TileChunks;
};

#define HANDMADE_TILE_H
#endif