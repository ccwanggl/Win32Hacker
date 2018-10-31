#include <windows.h>

LRESULT CALLBACK 
WindowProc(HWND Window,
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
        case WM_DESTROY:
        {
            OutputDebugString("WM_DESTORY\n");
        }break;
        case WM_CLOSE:
        {
			PostQuitMessage(0);
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
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            PatBlt(DeviceContext, X, Y, Width, Height,  WHITENESS);

            EndPaint(Window, &Paint);
        }break;
		default:
        {
            Result = DefWindowProc(Window, Message, wParam, lParam);
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
    WindowClass.lpfnWndProc     = WindowProc;
    WindowClass.hInstance       = hInstance; 
    WindowClass.lpszClassName   = "Handmade";

    if(RegisterClass(&WindowClass))
    {
        HWND WindowHandle =
            CreateWindowExA(
                  0,
                  WindowClass.lpszClassName,
				 "Handmade Hero",
                  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  0,
                  0,
                  hInstance,
                  0);
        if(WindowHandle)
        {
            for(;;)
            {
                MSG Message;
                BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
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
