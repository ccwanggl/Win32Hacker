#include <windows.h>
#include <stdint.h>


#define internal static 
#define local_persist static 
#define global_variable static 

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;



//TODO(guoliang): This is a global for now
global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;


global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;


internal void
RenderWeirdGradient(int BlueXOffset, int GreenYOffset)
{
	int Width = BitmapWidth;
	int Height = BitmapHeight;

	int Pitch = Width * BytesPerPixel;
	uint8 *Row = (uint8 *) BitmapMemory;
	for(int y = 0; y < BitmapHeight; y++)
	{
		uint32 *Pixel = (uint32 *) Row;
		for(int x = 0; x < BitmapWidth; x++)
		{
			/*
			 Pixel in memory: RR GG BB xx
			 0x xxBBGGRR

				Memory: BB GG RR xx
				Pixel:  xx RR GG BB
			 */
			uint8 Blue = (uint8)(x + BlueXOffset);
			uint8 Green = (uint8)(y + GreenYOffset);
			*Pixel ++ = ((Green << 8) | Blue);
		}
		Row += Pitch;
	}
}

internal void
Win32ResizeDIBSection(int Width, int Height)
{
    //TODO(guoliang): Bulletproof this.

	if(BitmapMemory)
	{
		VirtualFree( BitmapMemory, 0 , MEM_RELEASE);
	}

	BitmapWidth = Width;
	BitmapHeight = Height;


	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	//NOTE: Thank you to Chris Hecker of Spy party fame
	//
	//
	int BitmapMemorySize = (Width * Height) * BytesPerPixel;
	BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	RenderWeirdGradient(128, 0);

}

internal void
Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height)
{
	/*


	*/

	int WindowWidth = ClientRect->right - ClientRect->left;
	int WindowHeight = ClientRect->bottom - ClientRect->top;

	StretchDIBits(DeviceContext,
			0, 0, BitmapWidth, BitmapHeight,
			0, 0, WindowWidth, WindowHeight,
			BitmapMemory,
			&BitmapInfo,
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
			Win32ResizeDIBSection(Width, Height);
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


		RECT ClientRect;
		GetClientRect(Window, &ClientRect);
		Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);

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
        HWND Window=
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
        if(Window)
        {
		Running = true;
		int XOffset = 0;
		int YOffset = 0;
		    while(Running)
		    {
			MSG Message;
			while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
			{
				if(Message.message == WM_QUIT)
				{
					Running = false;
				}
			    TranslateMessage(&Message);
			    DispatchMessageA(&Message);
			}
			RenderWeirdGradient(XOffset, YOffset);

			HDC DeviceContext = GetDC(Window);
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			int WindowWidth = ClientRect.right - ClientRect.left;
			int WindowHeight = ClientRect.bottom - ClientRect.top;
			Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
			ReleaseDC(Window, DeviceContext);

			++XOffset;
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
