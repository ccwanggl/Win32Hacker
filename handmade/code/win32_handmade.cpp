#include <windows.h>

LRESULT CALLBACK 
windowProc(HWND Window,
           UINT Message,
           WPARAM wParam,
           LPARAM lParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_SIZE:
        {
            OutputDebugString("WM_SIZE\n");
        }break;
        case WM_DESTORY:
        {
            OutputDebugString("WM_DESTORY\n");
        }break;
        case WM_CLOSE:
        {
            OutputDebugString("WM_CLOSE\n");
        }break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugString("WM_ACTIVATEAPP\n");
        }break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - paint.rcPaint.top;
            PatBlt(DeviceContext, X, Y, Width, Height,  WHITENESS);

            EndPaint(Window, &Paint);
        }break;
        default
        {
            Result = DefWindowProc(window, message, wParam, lParam);
        }break;
    }
    return Result;
}
int WINAPI 
WinMain(HINSTANCE hInstance, 
		     HINSTANCE hPrevInstance, 
		     LPSTR lpCmdLine, 
		     int nCmdShow)
{
    WNDCLASS WindowClass = {};

    WindowClass.style           = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc     = ;
    WindowClass.hInstance       = hInstance; 
    WindowClass.lpszClassName   = "Handmade";

    if(RegisterClass(&windowclass))
    {
        HWND WindowHandle =
            CreateWindowEx(
                  0,
                  WindowClass.lpszClassName,
                  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                  WS_USEDEFAULT,
                  WS_USEDEFAULT,
                  WS_USEDEFAULT,
                  WS_USEDEFAULT,
                  0,
                  0,
                  hInstance,
                  0);
        if(WindowHandle)
        {
            for(;;)
            {
                MSG Message;
                BOOL WINAPI MessageResult = GetMessage(&Message, 0, 0, 0);
                if(MessageResult > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            //TODO
        }
    }
    else
    {
        //TODO
    }
    
	return 0;
}
