#include <math.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.14159265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#include "handmade.cpp"

#include <malloc.h>
#include <windows.h>
#include <stdio.h>
#include <xinput.h>
#include <dsound.h>

#include "win32_handmade.h"

global_variable bool GlobalRunning;
global_variable bool GlobalPause;
global_variable Win32_offscreen_buffer GlobalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
global_variable int64 GlobalPerfCountFrequency;

// XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) {return(ERROR_DEVICE_NOT_CONNECTED);}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) {return(ERROR_DEVICE_NOT_CONNECTED);}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

// dsound
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename){

    debug_read_file_result Result = {};

    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE){
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize)){
            uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if (Result.Contents){
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && ( FileSize32 == BytesRead)){
                    Result.ContentsSize = FileSize32;
                }
                else{
                    DEBUGPlatformFreeFileMemory(Result.Contents);
                    Result.Contents = 0;
                }
            }
        }
        CloseHandle(FileHandle);
    }
    return(Result);
}

internal void DEBUGPlatformFreeFileMemory(void *Memory){
    if(Memory){
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}
internal bool32 DEBUGPlatformWriteEntireFile(char *Filename, uint32 MemorySize, void *Memory){
    bool32 Result = false;

    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE){
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0)){
            Result = (BytesWritten == MemorySize);
        }
        else{

        }
        CloseHandle(FileHandle);
    }
    return(Result);
}

internal void Win32LoadXInput(void){
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if (!XInputLibrary){
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    if(XInputLibrary){
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetState) {XInputGetState = XInputGetStateStub;}
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetState) {XInputSetState = XInputSetStateStub;}
    }
}

internal void Win32InitSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize){
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if(DSoundLibrary){
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        WAVEFORMATEX WaveFormat = {};
        WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
        WaveFormat.nChannels = 2;
        WaveFormat.nSamplesPerSec = SamplesPerSecond;
        WaveFormat.wBitsPerSample = 16;
        WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
        WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
        WaveFormat.cbSize = 0;

        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))){

            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))){
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0))){

                    if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat))){

                    }
                }
            }
        }
        else{

        }
        DSBUFFERDESC BufferDescription = {};
        BufferDescription.dwSize = sizeof(BufferDescription);
        BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
        BufferDescription.dwBufferBytes = BufferSize;
        BufferDescription.lpwfxFormat = &WaveFormat;
        if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0))){

        }
    }
    else{

    }
}

internal win32_window_dimension Win32GetWindowDimension(HWND Window){

    win32_window_dimension Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}

internal void Win32ResizeDIBSection(Win32_offscreen_buffer *Buffer, int Width, int Height){

    if(Buffer->Memory){
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = ( Buffer->Width * Buffer->Height ) * Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);    

    Buffer->Pitch = Width * Buffer->BytesPerPixel;
}

internal void Win32DisplayBufferInWindow(Win32_offscreen_buffer *Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight){
    StretchDIBits(DeviceContext, 0, 0, WindowWidth, WindowHeight, 0, 0, Buffer->Width, Buffer->Height, Buffer->Memory, &Buffer->Info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam){
    LRESULT Result = 0;

    switch(Message){
        case WM_SIZE:{

        } break;

        case WM_CLOSE:{
            GlobalRunning = false;
        } break;

        case WM_ACTIVATEAPP:{
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_DESTROY: {
            GlobalRunning = false;
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:{
            Assert(!"Keyboard input came in through a non-dispatch message!");
        } break;

        case WM_PAINT:{
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);

            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;

            win32_window_dimension Dimention = Win32GetWindowDimension(Window);
            
            Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimention.Width ,Dimention.Height);
            EndPaint(Window, &Paint);
        }

        default:{
            // OutputDebugStringA("default\n");
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }
    return(Result);
}

internal void win32ClearBuffer(win32_sound_output *SoundOutput){
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;

    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize, &Region1, &Region1Size, &Region2, &Region2Size, 0))){
        int8 *DestSample = (int8 *)Region1;
        for(DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex){
            *DestSample++ = 0;
        }
        DestSample = (int8 *)Region2;
        for(DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex){
            *DestSample++ = 0;
        }
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite, game_sound_output_buffer *SourceBuffer){

    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;

    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0))){
        DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
        int16 *DestSample = (int16 *)Region1;
        int16 *SourceSample = SourceBuffer->Samples;
        for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex){
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
        DestSample = (int16 *)Region2;
        for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex){
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void Win32ProcessXInputDigitalButton(DWORD XInputButtonState, game_button_state *OldState, DWORD ButtonBit, game_button_state *NewState){
    NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
    

}

internal void Win32ProcessKeyboardMessage(game_button_state *NewState, bool32 IsDown){
    Assert(NewState->EndedDown != IsDown);
    NewState->EndedDown = IsDown;
    ++NewState->HalfTransitionCount;
}

internal float Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshhold){
    float Result = 0;

    if(Value < -DeadZoneThreshhold){
        Result = (float)((Value + DeadZoneThreshhold) / (32768.0f - DeadZoneThreshhold));
    }
    else if(Value > DeadZoneThreshhold){
        Result = (float)((Value - DeadZoneThreshhold) / (32767.0f - DeadZoneThreshhold));
    }
    return(Result);
}

internal void Win32ProcessPendingMessages(game_controller_input *KeyboardController){
    MSG Message;

    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)){
        switch(Message.message)
        {
            case WM_QUIT:{
                GlobalRunning = false;
            } break;

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:{
                uint32 VKCode = (uint32)Message.wParam;
                bool WasDown = ((Message.lParam & (1 << 30)) != 0);
                bool IsDown = ((Message.lParam & (1 << 31)) == 0);
                if(WasDown != IsDown){
                    if(VKCode == 'W'){
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
                    }else if(VKCode == 'A'){
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
                    }else if(VKCode == 'S'){
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
                    }else if(VKCode == 'D'){
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
                    }else if(VKCode == 'Q'){
                        Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
                    }else if(VKCode == 'E'){
                        Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
                    }else if(VKCode == VK_UP){
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown);
                    }else if(VKCode == VK_DOWN){
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown);
                    }else if(VKCode == VK_LEFT){
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionLeft, IsDown);
                    }else if(VKCode == VK_RIGHT){
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionRight, IsDown);
                    }else if(VKCode == VK_ESCAPE){
                        Win32ProcessKeyboardMessage(&KeyboardController->Start, IsDown);
                    }else if(VKCode == VK_SPACE){
                        Win32ProcessKeyboardMessage(&KeyboardController->Back, IsDown);
                    }
#if HANDMADE_INTERNAL
                    else if(VKCode == 'P'){
                        if(IsDown){
                            GlobalPause = !GlobalPause;
                        }
                    }
#endif
                }

                bool32 AltKeyWasDown = (Message.lParam & (1 <<29));
                if((VKCode == VK_F4) && AltKeyWasDown){     
                    GlobalRunning = false;
                }
            } break;
            default:{
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}

inline LARGE_INTEGER Win32GetWallClock(void){
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

inline float Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End){
    float Result = ((float)(End.QuadPart - Start.QuadPart) / (float)GlobalPerfCountFrequency);
    return(Result);
}

internal void Win32DebugDrawVertical(Win32_offscreen_buffer *BackBuffer, int X, int Top, int Bottom, uint32 Colour){

    if(Top <= 0){
        Top = 0;
    }

    if(Bottom > BackBuffer->Height){
        Bottom = BackBuffer->Height;
    }

    if((X >= 0) && (X < BackBuffer->Width)){
        uint8 *Pixel ((uint8 *)BackBuffer->Memory + (X * BackBuffer->BytesPerPixel) + (Top * BackBuffer->Pitch));
        for(int Y = Top; Y < Bottom; ++Y){
            *(uint32 *)Pixel = Colour;
            Pixel += BackBuffer->Pitch;
        }
    }
}

internal void Win32DrawSoundBufferMarker(Win32_offscreen_buffer *BackBuffer, win32_sound_output *SoundOutput, float C, int PadX, int Top, int Bottom, DWORD Value, uint32 Colour){
    float XFloat = (C * (float)Value);
    int X = PadX + (int)XFloat;

    Win32DebugDrawVertical(BackBuffer, X, Top, Bottom, Colour);
};

internal void Win32DebugSyncDisplay(Win32_offscreen_buffer *BackBuffer, int MarkerCount, win32_debug_time_marker *Markers, int CurrentMarkerIndex, win32_sound_output *SoundOutput, float TargetSecondsPerFrame){
    int PadX = 16;
    int PadY = 16;

    int LineHeight = 64;
    
    float C = (float)(BackBuffer->Width - 2 * PadX) / (float)SoundOutput->SecondaryBufferSize;
    for(int MarkerIndex = 0; MarkerIndex < MarkerCount; ++MarkerIndex){
        
        win32_debug_time_marker *ThisMarker = &Markers[MarkerIndex];
        Assert(ThisMarker->OutputPlayCursor < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->OutputWriteCursor < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->OutputLocation < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->OutputByteCount < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->FlipPlayCursor < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->FlipWriteCursor < SoundOutput->SecondaryBufferSize);

        DWORD PlayColour = 0xFFFFFFFF;
        DWORD WriteColour = 0xFFFF0000;
        DWORD ExpectedFlipColour = 0xFFFFF00;
        DWORD PlayWindowColour = 0xFFFF00FF;
        
        int Top = PadY;
        int Bottom = PadY + LineHeight;
        if(MarkerIndex == CurrentMarkerIndex){

            Top += LineHeight + PadY;
            Bottom += LineHeight + PadY;

            int FirstTop = Top;

            Win32DrawSoundBufferMarker(BackBuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputPlayCursor, PlayColour);
            Win32DrawSoundBufferMarker(BackBuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputWriteCursor, WriteColour);

            Top += LineHeight + PadY;
            Bottom += LineHeight + PadY;
            
            Win32DrawSoundBufferMarker(BackBuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputLocation, PlayColour);
            Win32DrawSoundBufferMarker(BackBuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputLocation + ThisMarker->OutputByteCount, WriteColour);
            
            
            Top += LineHeight + PadY;
            Bottom += LineHeight + PadY;

            Win32DrawSoundBufferMarker(BackBuffer, SoundOutput, C, PadX, FirstTop, Bottom, ThisMarker->ExpectedFlipPlayCursor, ExpectedFlipColour);
        }

        Win32DrawSoundBufferMarker(BackBuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipPlayCursor, PlayColour);
        Win32DrawSoundBufferMarker(BackBuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipPlayCursor + 480*SoundOutput->BytesPerSample, PlayWindowColour);
        Win32DrawSoundBufferMarker(BackBuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipWriteCursor, WriteColour);
    }
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode){

    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    // NOTE: Set the Windows scheduler granularity to 1ms so that Sleep() can be more granular.
    UINT DesiredSchedulerMS = 1;
    bool32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);

    Win32LoadXInput();

    WNDCLASSA WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackBuffer ,1280, 720);

    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    // HICON hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    #define MonitorRefreshHz 60
    #define GameUpdateHz (MonitorRefreshHz / 2)
    float TargetSecondsPerFrame = 1.0f / (float)GameUpdateHz;

    if(RegisterClassA(&WindowClass)){
        HWND Window = CreateWindowExA(0, WindowClass.lpszClassName, "Handmake Hero", WS_OVERLAPPEDWINDOW|WS_VISIBLE|CS_OWNDC, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0); 

        if(Window){
            HDC DeviceContext = GetDC(Window);
            win32_sound_output SoundOutput = {};

            // TODO: Make this like sixty seconds?
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.BytesPerSample = sizeof(int16) * 2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
            // TODO: Get rid of LatencySampleCount
            SoundOutput.LatencySampleCount = 3 * (SoundOutput.SamplesPerSecond / GameUpdateHz);
            // TODO: Actually conpute this variance and see
            // what is lowest reasonable value is.
            SoundOutput.SafetyBytes = (SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample / GameUpdateHz) / 3;
            Win32InitSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            win32ClearBuffer(&SoundOutput);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            GlobalRunning = true;

            int16 *Samples = (int16 *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

#if HANDMADE_INTERNAL
            LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
            LPVOID BaseAddress = 0;
#endif

            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabytes(64);
            GameMemory.TransientStorageSize = Gigabytes((uint64)4);

            uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
            GameMemory.PermanentStorage = VirtualAlloc(BaseAddress, (size_t)TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            GameMemory.TransientStorage = ((uint8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);

            if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage){

                game_input Input[2] = {};
                game_input *NewInput = &Input[0];
                game_input *OldInput = &Input[1];

                LARGE_INTEGER LastCounter = Win32GetWallClock();
                LARGE_INTEGER FlipWallClock = Win32GetWallClock();

                int DebugTimeMarkerIndex = 0;
                win32_debug_time_marker DebugTimeMarkers[GameUpdateHz / 2] = {0};

                DWORD AudioLatencyBytes = 0;
                float AudioLatencySeconds = 0;
                bool32 SoundIsValid = false;

                uint64 LastCycleCount = __rdtsc();
                while(GlobalRunning){

                    game_controller_input *OldKeyboardController = GetController(OldInput, 0);
                    game_controller_input *NewKeyboardController = GetController(NewInput, 0);
                    *NewKeyboardController = {};;
                    NewKeyboardController->IsConnected = true;

                    for(int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ++ButtonIndex){
                        NewKeyboardController->Buttons[ButtonIndex].EndedDown = OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                    }

                    Win32ProcessPendingMessages(NewKeyboardController);

                    if(!GlobalPause){
                        DWORD MaxControllerCount = XUSER_MAX_COUNT;
                        if(MaxControllerCount > ArrayCount(NewInput->Controllers) - 1){
                            MaxControllerCount = ArrayCount(NewInput->Controllers) - 1;
                        }
                        for(DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ControllerIndex++){

                            DWORD OurControllerIndex = ControllerIndex + 1;
                            game_controller_input *OldController = GetController(OldInput, OurControllerIndex);
                            game_controller_input *NewController = GetController(NewInput, OurControllerIndex);

                            XINPUT_STATE ControllerState;
                            if(XInputGetState_(ControllerIndex,&ControllerState) == ERROR_SUCCESS){
                                NewController->IsConnected = true;
                                XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                                NewController->StickAverageX = Win32ProcessXInputStickValue(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                                NewController->StickAverageY = Win32ProcessXInputStickValue(Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                                if((NewController->StickAverageX != 0.0f) || (NewController->StickAverageY != 0.0f)){
                                    NewController->IsAnalog = true;
                                }

                                if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP){
                                    NewController->StickAverageY = 1.0f;
                                    NewController->IsAnalog = false;
                                }
                                if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN){
                                    NewController->StickAverageY = -1.0f;
                                    NewController->IsAnalog = false;
                                }
                                if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT){
                                    NewController->StickAverageX = -1.0f;
                                    NewController->IsAnalog = false;
                                }
                                if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT){
                                    NewController->StickAverageX = 1.0f;
                                    NewController->IsAnalog = false;
                                }

                                float Threshold = 0.5f;
                                Win32ProcessXInputDigitalButton((NewController->StickAverageX < -Threshold) ? 1 : 0, &OldController->MoveLeft, 1, &NewController->MoveLeft);
                                Win32ProcessXInputDigitalButton((NewController->StickAverageX > Threshold) ? 1 : 0, &OldController->MoveRight, 1, &NewController->MoveRight);
                                Win32ProcessXInputDigitalButton((NewController->StickAverageY < -Threshold) ? 1 : 0, &OldController->MoveDown, 1, &NewController->MoveDown);
                                Win32ProcessXInputDigitalButton((NewController->StickAverageY > Threshold) ? 1 : 0, &OldController->MoveUp, 1, &NewController->MoveUp);

                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->ActionDown, XINPUT_GAMEPAD_A, &NewController->ActionDown);
                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->ActionRight, XINPUT_GAMEPAD_B, &NewController->ActionRight);
                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->ActionLeft, XINPUT_GAMEPAD_X, &NewController->ActionLeft);
                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->ActionUp, XINPUT_GAMEPAD_Y, &NewController->ActionUp);
                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER, &NewController->LeftShoulder);
                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER, &NewController->RightShoulder);
                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->Start, XINPUT_GAMEPAD_START, &NewController->Start);
                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->Back, XINPUT_GAMEPAD_BACK, &NewController->Back);
                            }
                            else{
                                NewController->IsConnected = false;
                            }
                        }

                        game_offscreen_buffer Buffer = {};
                        Buffer.Memory = GlobalBackBuffer.Memory;
                        Buffer.Width = GlobalBackBuffer.Width;
                        Buffer.Height = GlobalBackBuffer.Height;
                        Buffer.Pitch = GlobalBackBuffer.Pitch;
                        GameUpdateAndRender(&GameMemory, NewInput, &Buffer);

                        LARGE_INTEGER AudioWallClock = Win32GetWallClock();
                        float FromBeginToAudioSeconds = Win32GetSecondsElapsed(FlipWallClock, AudioWallClock);

                        DWORD PlayCursor;
                        DWORD WriteCursor;
                        if(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK) {

                            if(!SoundIsValid){

                                SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
                                SoundIsValid = true;
                            }
                            
                            DWORD ByteToLock = ((SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize);

                            DWORD ExpectedSoundBytesPerFrame = ((SoundOutput.SamplesPerSecond * SoundOutput.SamplesPerSecond) / GameUpdateHz);
                            float SecondsLeftUntilFlip = (TargetSecondsPerFrame - FromBeginToAudioSeconds);
                            DWORD ExpectedBytesUntilFlip = (DWORD)((SecondsLeftUntilFlip/TargetSecondsPerFrame) * (float)ExpectedSoundBytesPerFrame);
                            DWORD ExpectedFrameBoundryByte = (PlayCursor + ExpectedSoundBytesPerFrame);

                            DWORD SafeWriteCursor = WriteCursor;
                            if(SafeWriteCursor < PlayCursor){
                                SafeWriteCursor += SoundOutput.SecondaryBufferSize;
                            }
                            Assert(SafeWriteCursor >= PlayCursor);
                            SafeWriteCursor += SoundOutput.SafetyBytes;
                            bool32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundryByte);

                            DWORD TargetCursor = 0;
                            if(AudioCardIsLowLatency){
                                TargetCursor = (ExpectedFrameBoundryByte + ExpectedSoundBytesPerFrame);
                            }
                            else{
                                TargetCursor = (WriteCursor + ExpectedSoundBytesPerFrame + SoundOutput.SafetyBytes);
                            }
                            TargetCursor = (TargetCursor % SoundOutput.SecondaryBufferSize);

                            DWORD BytesToWrite = 0;
                            if(ByteToLock > TargetCursor){
                                BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
                                BytesToWrite += TargetCursor;
                            }
                            else{
                                BytesToWrite = TargetCursor - ByteToLock;
                            }
                            
                            game_sound_output_buffer SoundBuffer = {};
                            SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                            SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                            SoundBuffer.Samples = Samples;
                            GameGetSoundSamples(&GameMemory, &SoundBuffer);
#if HANDMADE_INTERNAL
                            win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
                            Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
                            Marker->OutputPlayCursor = PlayCursor;
                            Marker->OutputWriteCursor = WriteCursor;
                            Marker->OutputLocation = ByteToLock;
                            Marker->OutputByteCount = BytesToWrite;
                            Marker->ExpectedFlipPlayCursor = ExpectedFrameBoundryByte;

                            DWORD UnwrappedWriteCursor = WriteCursor;
                            if(UnwrappedWriteCursor < PlayCursor){
                                UnwrappedWriteCursor += SoundOutput.SecondaryBufferSize;
                            }
                            AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;
                            AudioLatencySeconds = (((float)AudioLatencyBytes / (float)SoundOutput.BytesPerSample) / (float)SoundOutput.SamplesPerSecond);

                            char TextBuffer[256];
                            _snprintf_s(TextBuffer, sizeof(TextBuffer), "BTL:%u TC:%u BTW:%u - PC:%u WC:%u DELTA:%u (%fs)\n ",ByteToLock, TargetCursor, BytesToWrite, PlayCursor, WriteCursor, AudioLatencyBytes, AudioLatencySeconds);
                            OutputDebugStringA(TextBuffer);
#endif
                            win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
                        }
                        else{
                            SoundIsValid = false;
                        }

                        LARGE_INTEGER WorkCounter = Win32GetWallClock();
                        float WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);

                        float SecondsElapsedForFrame = WorkSecondsElapsed;
                        if(SecondsElapsedForFrame < TargetSecondsPerFrame){
                            if(SleepIsGranular){
                                DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
                                if(SleepMS > 0){
                                    Sleep(SleepMS);
                                }
                            }

                            float TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
                            if(TestSecondsElapsedForFrame < TargetSecondsPerFrame){
                                //LOG MISSED SLEEP HERE
                            }

                            while(SecondsElapsedForFrame < TargetSecondsPerFrame){
                                SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
                            }
                        }
                        else{
                            // MISSED FRAME RATE
                        }

                        LARGE_INTEGER EndCounter = Win32GetWallClock();
                        float MSPerFrame = 1000.0f * Win32GetSecondsElapsed(LastCounter, EndCounter);
                        LastCounter = EndCounter;
                        
                        win32_window_dimension Dimention = Win32GetWindowDimension(Window);
#if HANDMADE_INTERNAL
                        Win32DebugSyncDisplay(&GlobalBackBuffer, ArrayCount(DebugTimeMarkers), DebugTimeMarkers, DebugTimeMarkerIndex - 1, &SoundOutput, TargetSecondsPerFrame);
#endif
                        Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimention.Width, Dimention.Height);

                        FlipWallClock = Win32GetWallClock();
#if HANDMADE_INTERNAL
                        // NOTE: This is debug code
                        {
                            DWORD PlayCursor;
                            DWORD WriteCursor;
                            if(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK) {
                                Assert(DebugTimeMarkerIndex < ArrayCount(DebugTimeMarkers))
                                win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
                                Marker->FlipPlayCursor = PlayCursor;
                                Marker->FlipWriteCursor = WriteCursor;
                            }
                        }
#endif

                        game_input *Temp = NewInput;
                        NewInput = OldInput;
                        OldInput = Temp;

                        uint64 EndCycleCount = __rdtsc();
                        uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
                        LastCycleCount = EndCycleCount;

                        float FPS = 0.0f;
                        float MCPF = ((float)CyclesElapsed / (1000.0f * 1000.0f));

                        char FPSBuffer[256];
                        _snprintf_s(FPSBuffer, sizeof(FPSBuffer), "%.02fms/f - %.02fFPS - %.02fmc/f\n", MSPerFrame, FPS, MCPF);
                        OutputDebugStringA(FPSBuffer);

#if HANDMADE_INTERNAL
                        ++DebugTimeMarkerIndex;
                        if(DebugTimeMarkerIndex == ArrayCount(DebugTimeMarkers)){
                            DebugTimeMarkerIndex = 0;
                        }
#endif
                    }
                }
            }else{

            }
        }
        else{

        }
    }
    else{

    }
    return(0);
}