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
internal int32 RoundReal32ToInt32(float Real32)
{
    int32 Result = (int32)(Real32 + 0.5f);
    return(Result);
}

internal uint32 RoundReal32ToUInt32(float Real32)
{
    uint32 Result = (uint32)(Real32 + 0.5f);
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

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) == (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(!Memory->IsInitialised)
    {
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

            GameState->PlayerX += dPlayerX * Input->dtForFrame;
            GameState->PlayerY += dPlayerY * Input->dtForFrame;
        }
    }

    uint32 TileMap[9][17]=
    {
        {0, 0, 0, 0,  0, 0, 0, 0,  1,  0, 0, 0, 0,  0, 0, 0, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  0,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  0,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  0,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 1, 1, 1,  1, 1, 1, 1,  1,  1, 1, 1, 1,  1, 1, 1, 0},
        {0, 0, 0, 0,  0, 0, 0, 0,  1,  0, 0, 0, 0,  0, 0, 0, 0}
    };

    float UpperLeftX = -30;
    float UpperLeftY = 0;
    float TileWidth = 60;
    float TileHeight = 60;

    DrawRectangle(Buffer, 0.0f, 0.0f, (float)Buffer->Width, (float)Buffer->Height, 1.0f, 0.0f, 0.9f);

    for(int Row = 0; Row < 9; ++Row)
    {
        for(int Column = 0; Column < 17; ++Column)
        {
            uint32 TileID = TileMap[Row][Column];
            float Gray = 0.5f;
            if(TileID == 1)
            {
                Gray = 1.0f;
            }

            // float32_t
            float MinX = UpperLeftX + ((float)Column) * TileWidth;
            float MinY = UpperLeftY + ((float)Row) * TileWidth;
            float MaxX = MinX + TileWidth;
            float MaxY = MinY + TileWidth;

            DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
        }
    }

    float PlayerR = 1.0f;
    float PlayerG = 1.0f;
    float PlayerB = 0.0f;
    float PlayerWidth = 0.75f * TileWidth;
    float PlayerHeight = TileHeight;
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