#include <windows.h>

LRESULT CALLBACK
MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam){
    LRESULT Result = 0;

    switch(Message){
        case WM_SIZE:{
            OutputDebugStringA("WM_SIZE\n");
        }break;

        case WM_DESTROY:{
            OutputDebugStringA("WM_DESTROY\n");
        }break;

        case WM_CLOSE:{
            OutputDebugStringA("WM_CLOSE\n");
        }break;

        case WM_ACTIVATEAPP:{
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }break;

        case WM_PAINT:{
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            static DWORD Operation = WHITENESS;
            PatBlt(DeviceContext, X,Y, Width, Height, Operation);
            if(Operation == WHITENESS){
                Operation = BLACKNESS;
            }
            else{
                Operation = WHITENESS;
            }
            EndPaint(Window, &Paint);
        }

        default:{
            OutputDebugStringA("default\n");
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        }break;
    }
    return(Result);
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode){
    WNDCLASS WindowClass = {};
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
    // HICON hIcon;
    WindowClass.lpszClassName = "Handmade Hero Window Class";

    if(RegisterClassA(&WindowClass)){
        HWND WindowHandle =
            CreateWindowExA(0, WindowClass.lpszClassName, "Handmake Hero", WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0); 
        if(WindowHandle){
            for(;;){
                MSG Message;
                BOOL MEssageResult = GetMessageA(&Message, 0, 0,0);
                if(MEssageResult > 0){
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                else{
                    break;
                }
            }
        }
        else{

        }
    }
    else{

    }
    return(0);
}