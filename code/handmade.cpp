#include "handmade.h"

internal void GameOutputSound(game_state *GameState ,game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    int16 ToneVolume = 2000;
    int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

    int16 *SampleOut = SoundBuffer->Samples;

    for(int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
#if 0
        float SineValue = sinf(GameState->tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
#else
        int16 SampleValue = 0;
#endif
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
#if 0
        GameState->tSine += 2.0f * Pi32 * 1.0f / (float)WavePeriod;
        if(GameState->tSine > 2.0f * Pi32)
        {
            GameState->tSine -= 2.0f * Pi32;
        }
#endif
    }
}
inline int32 RoundReal32ToInt32(float Real32)
{
    int32 Result = (int32)(Real32 + 0.5f);
    return(Result);
}

inline uint32 RoundReal32ToUInt32(float Real32)
{
    uint32 Result = (uint32)(Real32 + 0.5f);
    return(Result);
}

inline int32 TruncateReal32ToInt32(float Real32)
{
    int32 Result = (int32)Real32;
    return(Result);
}

internal void DrawRectangle(game_offscreen_buffer *Buffer, float RealMinX, float RealMinY, float RealMaxX, float RealMaxY, float R, float G, float B)
{
    int32 MinX = RoundReal32ToInt32(RealMinX);
    int32 MinY = RoundReal32ToInt32(RealMinY);
    int32 MaxX = RoundReal32ToInt32(RealMaxX);
    int32 MaxY = RoundReal32ToInt32(RealMaxY);

    if(MinX < 0)
    {
        MinX = 0;
    }

    if(MinY < 0)
    {
        MinY = 0;
    }

    if(MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }

    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }

    uint32 Colour = ((RoundReal32ToUInt32(R * 255.0f) << 16) |
                     (RoundReal32ToUInt32(G * 255.0f) << 8 ) |
                     (RoundReal32ToUInt32(B * 255.0f) << 0 ));

    uint8 *Row ((uint8 *)Buffer->Memory + (MinX * Buffer->BytesPerPixel) + (MinY * Buffer->Pitch));

    for(int Y = MinY; Y < MaxY; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = MinX; X < MaxX; ++X)
        {
            *Pixel++ = Colour;
        }

        Row += Buffer->Pitch;
    }
}

inline tile_map * GetTileMap(world *World, int32 TileMapX, int32 TileMapY)
{
    tile_map *TileMap = 0;

    if((TileMapX >= 0) && (TileMapX < World->TileMapCountX) &&
       (TileMapY >= 0) && (TileMapY < World->TileMapCountY))
    {
        TileMap = &World->TileMaps[TileMapY * World->TileMapCountX + TileMapX];
    }
    return(TileMap);
}

inline uint32 GetTileValueUnchecked(tile_map *TileMap, int32 TileX, int32 TileY)
{
    uint32 TileMapValue = TileMap->Tiles[TileY * TileMap->CountX + TileX];
    return(TileMapValue);
}

internal bool32 IsTileMapPointEmpty(tile_map *TileMap, float TestX, float TestY)
{
    bool32 Empty = false;

    int32 PlayerTileX = TruncateReal32ToInt32((TestX - TileMap->UpperLeftX) / TileMap->TileWidth);
    int32 PlayerTileY = TruncateReal32ToInt32((TestY - TileMap->UpperLeftY) / TileMap->TileHeight);

    if((PlayerTileX >= 0) && (PlayerTileX < TileMap->CountX) &&
       (PlayerTileY >= 0) && (PlayerTileY < TileMap->CountY))
    {
        uint32 TileMapValue = GetTileValueUnchecked(TileMap, PlayerTileX, PlayerTileY);
        Empty = (TileMapValue == 1);
    }

    return(Empty);
}

internal bool32 IsWorldPointEmpty(world *World, int32 TileMapX, int32 TileMapY, float TestX, float TestY)
{
    bool32 Empty = false;

    tile_map *TileMap = GetTileMap(World, TileMapX, TileMapY);
    if(TileMap)
    {
        int32 PlayerTileX = TruncateReal32ToInt32((TestX - TileMap->UpperLeftX) / TileMap->TileWidth);
        int32 PlayerTileY = TruncateReal32ToInt32((TestY - TileMap->UpperLeftY) / TileMap->TileHeight);

        if((PlayerTileX >= 0) && (PlayerTileX < TileMap->CountX) &&
        (PlayerTileY >= 0) && (PlayerTileY < TileMap->CountY))
        {
            uint32 TileMapValue = GetTileValueUnchecked(TileMap, PlayerTileX, PlayerTileY);
            Empty = (TileMapValue == 1);
        }
    }
    return(Empty);
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) == (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

#define TILE_MAP_COUNT_X 17
#define TILE_MAP_COUNT_Y 9

    uint32 Tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {0, 0, 0, 0,  0, 0, 0, 0,  1,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 0, 0, 0,  0, 0, 0, 0,  1,  0, 0, 0, 0,  0, 0, 0, 0}
    };

    uint32 Tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {0, 0, 0, 0,  0, 0, 0, 0,  1,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  0,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 0, 0},
        {0, 0, 0, 0,  0, 0, 0, 0,  1,  0, 0, 0, 0,  0, 0, 0, 0}
    };
    uint32 Tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {0, 0, 0, 0,  0, 0, 0, 0,  1,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  0,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 0, 0},
        {0, 0, 0, 0,  0, 0, 0, 0,  1,  0, 0, 0, 0,  0, 0, 0, 0}
    };
    uint32 Tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {0, 0, 0, 0,  0, 0, 0, 0,  1,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 0, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  0,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 0, 0},
        {0, 0, 0, 0,  0, 0, 0, 0,  1,  0, 0, 0, 0,  0, 0, 0, 0}
    };

    tile_map TileMaps[2][2];
    TileMaps[0][0].CountX = TILE_MAP_COUNT_X;
    TileMaps[0][0].CountY = TILE_MAP_COUNT_Y;

    TileMaps[0][0].UpperLeftX = -30;
    TileMaps[0][0].UpperLeftY = 0;
    TileMaps[0][0].TileWidth = 60;
    TileMaps[0][0].TileHeight = 60;

    TileMaps[0][0].Tiles = (uint32 *)Tiles00;

    TileMaps[0][1] = TileMaps[0][0];
    TileMaps[0][1].Tiles = (uint32 *)Tiles01;

    TileMaps[1][0] = TileMaps[0][0];
    TileMaps[1][0].Tiles = (uint32 *)Tiles10;

    TileMaps[1][1] = TileMaps[0][0];
    TileMaps[1][1].Tiles = (uint32 *)Tiles11;

    tile_map *TileMap = &TileMaps[0][0];

    world World;
    World.TileMapCountX = 2;
    World.TileMapCountY = 2;

    World.TileMaps = (tile_map *)TileMaps;

    float PlayerWidth = 0.75f * TileMap->TileWidth;
    float PlayerHeight = TileMap->TileHeight;

    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(!Memory->IsInitialised)
    {
        GameState->PlayerX = 150;
        GameState->PlayerY = 150;
        Memory->IsInitialised = true;
    }

    for(int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        if(Controller->IsAnalog)
        {
            // NOTE: Use analog movement tuning
        }
        else
        {
            // NOTE: Use digital movement tuning
            float dPlayerX = 0.0f;
            float dPlayerY = 0.0f;

            if(Controller->MoveUp.EndedDown)
            {
                dPlayerY = -1.0f;
            }

            if(Controller->MoveDown.EndedDown)
            {
                dPlayerY = 1.0f;
            }

            if(Controller->MoveLeft.EndedDown)
            {
                dPlayerX = -1.0f;
            }

            if(Controller->MoveRight.EndedDown)
            {
                dPlayerX = 1.0f;
            }
            dPlayerX *= 64.0f;
            dPlayerY *= 64.0f;

            float NewPlayerX = GameState->PlayerX + dPlayerX * Input->dtForFrame;
            float NewPlayerY = GameState->PlayerY + dPlayerY * Input->dtForFrame;

            if(IsTileMapPointEmpty(TileMap, NewPlayerX - 0.5f * PlayerWidth, NewPlayerY) &&
                IsTileMapPointEmpty(TileMap, NewPlayerX + 0.5f * PlayerWidth, NewPlayerY) &&
                IsTileMapPointEmpty(TileMap, NewPlayerX, NewPlayerY))
            {
                GameState->PlayerX = NewPlayerX;
                GameState->PlayerY = NewPlayerY;
            }
        }
    }

    DrawRectangle(Buffer, 0.0f, 0.0f, (float)Buffer->Width, (float)Buffer->Height, 1.0f, 0.0f, 0.9f);

    for(int Row = 0; Row < 9; ++Row)
    {
        for(int Column = 0; Column < 17; ++Column)
        {
            uint32 TileID = GetTileValueUnchecked(TileMap, Column, Row);
            float Gray = 0.5f;
            if(TileID == 1)
            {
                Gray = 1.0f;
            }

            float MinX = TileMap->UpperLeftX + ((float)Column) * TileMap->TileWidth;
            float MinY = TileMap->UpperLeftY + ((float)Row) * TileMap->TileWidth;
            float MaxX = MinX + TileMap->TileWidth;
            float MaxY = MinY + TileMap->TileWidth;

            DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
        }
    }

    float PlayerR = 1.0f;
    float PlayerG = 1.0f;
    float PlayerB = 0.0f;

    float PlayerLeft = GameState->PlayerX - (0.5f * PlayerWidth);
    float PlayerTop = GameState->PlayerY - PlayerHeight;
    
    DrawRectangle(Buffer, PlayerLeft, PlayerTop, PlayerLeft + PlayerWidth, PlayerTop + PlayerHeight, PlayerR, PlayerG, PlayerB);

    DrawRectangle(Buffer, 10.0f, 10.0f, 30.0f, 30.0f, 1.0f, 1.0f, 0.0f);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    GameOutputSound(GameState, SoundBuffer, 400);
}

// internal void RenderWeirdGradient(game_offscreen_buffer *Buffer, int BlueOffSet, int GreenOffSet){
//     uint8 *Row = (uint8 *)Buffer->Memory;
//     for(int Y = 0; Y < Buffer->Height; ++Y){
//         uint32 *Pixel = (uint32 *)Row;
//         for(int X = 0; X < Buffer->Width; ++X){

//             uint8 Blue = (uint8)(X + BlueOffSet);
//             uint8 Green = (uint8)(Y + GreenOffSet);

//             *Pixel++ = ((Green << 8) | Blue);
//         }
//         Row += Buffer->Pitch;
//     }
// }