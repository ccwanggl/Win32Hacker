#include <windows.h>


#define internal static 
#define local_persist static 
#define global_variable static 

//TODO(guoliang): This is a global for now
global_variable bool Running;


global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;

internal void
win32ResizeDIBSection(int Width, int Height)
{
	if (BitmapHandle)
	{
		DeleteObject(BitmapHandle);
	}

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = Height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biSizeImage = 0;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biClrUsed = 0;
	BitmapInfo.bmiHeader.biClrImportant = 0;

	HDC DeviceContext = GetCompatibleDC(0);

	BitmapHandle = CreateDIBSection(DeviceContext,
		&BitmapInfo,
		DIB_RGB_COLORS,
		&BitmapMemory,
		0, 0);
	ReleaseDC(0, DeviceContext);
}

internal void
win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
	StretchDIBits(DeviceContext,
		X, Y, Width, Height,
		X, Y, Width, Height,

		DIB_RGB_COLORS,
		SRCCOPY);
}

LRESULT CALLBACK 
win32WindowProc(HWND Window,
           UINT Message,
           WPARAM wParam,
           LPARAM lParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_SIZE:
        {
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			int Width = ClientRect.right - ClientRect.left;
			int Height = ClientRect.bottom - ClientRect.top;
			win32ResizeDIBSection(Width, Height);
        }break;
        case WM_DESTROY:
        {
			Running = false;
			//TODO: Handle this as an error - recreate window?
        }break;
        case WM_CLOSE:
        {
			//TODO: Handle this with a message to the user?
			Running = false;
        }break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			win32UpdateWindow(DeviceContext, X, Y, Width, Height);
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
    WindowClass.lpfnWndProc     = win32WindowProc;
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
			Running = true;
            while(Running)
            {
                MSG Message;
                BOOL MessageResult = GetMessageA(&Message, 0, 0, 0);
                if(MessageResult > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
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
