#include <windows.h>
#include <stdint.h>
#include <Xinput.h>


#define internal			static
#define local_persist		static
#define global_variable		static

typedef int8_t			int8;
typedef int16_t			int16;
typedef int32_t			int32;
typedef int64_t			int64;

typedef uint8_t			uint8;
typedef uint16_t		uint16;
typedef uint32_t		uint32;
typedef uint64_t		uint64;

struct win32_offscreen_buffer
{
	BITMAPINFO	Info;
	void		*Memory;
	int		Width;
	int		Height;
	int		Pitch;
	int		BytesPerPixel;
};

struct win32_window_dimension
{
	int Width;
	int Height;
};


//NOTE(guoliang): XInputGetState
#define  X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);

X_INPUT_GET_STATE(XInputGetStateStub)
{
	return (0);
}

global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

//NOTE(guoliang): XInputSetState
#define  X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return (0);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_


//TODO(guoliang): This is a global for now
global_variable bool					GlobalRunning;
global_variable win32_offscreen_buffer	GlobalBackbuffer;

win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension	Result;
	RECT					ClientRect;


	GetClientRect(Window, &ClientRect);
	Result.Width	=	ClientRect.right - ClientRect.left;
	Result.Height	=	ClientRect.bottom - ClientRect.top;

	return Result;
};

internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer, int BlueXOffset, int GreenYOffset)
{
	uint8 *Row = (uint8 *) Buffer.Memory;
	for(int y = 0; y < Buffer.Height; y++)
	{
		uint32 *Pixel = (uint32 *) Row;
		for(int x = 0; x < Buffer.Width; x++)
		{
			/*
			 Pixel in memory: RR GG BB xx
			 0x xxBBGGRR

				Memory: BB GG RR xx
				Pixel:  xx RR GG BB
			 */

			uint8 Blue	= (uint8)(x + BlueXOffset);
			uint8 Green = (uint8)(y + GreenYOffset);
			*Pixel++	= ((Green << 8) | Blue);
		}
		Row += Buffer.Pitch;
	}
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    //TODO(guoliang): Bulletproof this.
    //Maybe don't free first, free after, then free first of that fails.

	if(Buffer->Memory)
	{
		VirtualFree( Buffer->Memory, 0 , MEM_RELEASE);
	}

	Buffer->Width			= Width;
	Buffer->Height			= Height;
	Buffer->BytesPerPixel	= 4;

	//NOTE(guoliang): When the biHeight field is negative. this is the clue to windows to treat
	//this bitmap as top_down, not bottom-up. meaning that the first three bytes of the image are
	//the color for the top left pixel in the bitmap, not the bottom left.

	Buffer->Info.bmiHeader.biSize			= sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth			= Buffer->Width;
	Buffer->Info.bmiHeader.biHeight			= -Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes			= 1;
	Buffer->Info.bmiHeader.biBitCount		= 32;
	Buffer->Info.bmiHeader.biCompression	= BI_RGB;

	//NOTE: Thank you to Chris Hecker of Spy party fame
	//for claritying the deal with StretchDIBits and BitBle!

	int BitmapMemorySize	= (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
	Buffer->Memory			= VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	Buffer->Pitch			= Width * Buffer->BytesPerPixel;


	//TODO(guoliang): Probably clear this to black
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext,
			   int WindowWidth,
			   int WindowHeight,
			   win32_offscreen_buffer Buffer
			   )
{
	//TODO(guoliang): Aspect ratio correction
	StretchDIBits(DeviceContext,
			0, 0, WindowWidth, WindowHeight, 			// rectangle wants to be full
			0, 0, Buffer.Width, Buffer.Height,			// buffer size
			Buffer.Memory,                              // buffer address
			&Buffer.Info,                               // BITMAPINFO
			DIB_RGB_COLORS,
			SRCCOPY
		);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
           UINT Message,
           WPARAM wParam,
           LPARAM lParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_SIZE:
        {
        }break;
        case WM_CLOSE:
        {
			//TODO(guoliang): Handle this with a message to the user?
			GlobalRunning = false;
        }break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }break;
        case WM_DESTROY:
        {
			GlobalRunning = false;
			//TODO(guoliang): Handle this as an error - recreate window?
        }break;
        case WM_PAINT:
        {
		    PAINTSTRUCT Paint;

		    HDC DeviceContext = BeginPaint(Window, &Paint);
		    int X = Paint.rcPaint.left;
		    int Y = Paint.rcPaint.top;
		    int Width	= Paint.rcPaint.right - Paint.rcPaint.left;
		    int Height	= Paint.rcPaint.bottom - Paint.rcPaint.top;

		    win32_window_dimension Dimension = Win32GetWindowDimension(Window);
		    Win32DisplayBufferInWindow(DeviceContext,
				    Dimension.Width, Dimension.Height,
				    GlobalBackbuffer);

            EndPaint(Window, &Paint);
        }break;
		default:
        {
            Result = DefWindowProc(Window, Message, wParam, lParam);
        }break;
    }
    return Result;
}

int CALLBACK
WinMain(HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     LPSTR lpCmdLine,
		     int nCmdShow)
{
	//TODO(guoliang): stack overflow
	// uint8 BigOldBlockOfMemory[2 * 1024 * 1024] ={};

    WNDCLASS WindowClass{};

    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

    WindowClass.style           = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc     = Win32MainWindowCallback;
    WindowClass.hInstance       = hInstance;
    WindowClass.lpszClassName   = "HandmadeHeroWindowClass";

    if(RegisterClass(&WindowClass))
    {
		HWND Window= CreateWindowExA(
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
			//NOTE(guoliang): Since we specified CS_OWNDC, we can just get one device context and use it forever
			// because we are note sharing it with anyone.
			
			HDC DeviceContext = GetDC(Window);
			GlobalRunning = true;

			int XOffset = 0;
			int YOffset = 0;

		    while(GlobalRunning)
		    {
				MSG Message;
				while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
				{
					if(Message.message == WM_QUIT)
					{
						GlobalRunning = false;
					}
					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}

				// TODO(guoliang): Should we poll this more frequently
				for(DWORD ControllerIndex = 0;
							ControllerIndex < XUSER_MAX_COUNT;
							++ControllerIndex)
				{
					XINPUT_STATE ControllerStatus;
					if(XInputGetState(ControllerIndex, &ControllerStatus) == ERROR_SUCCESS)
					{
						// NOTE(guoliang): This controller is plugged in 
						// TODO(guoliang): See if ControllerState.dwPacketNumber increments rapidly
						XINPUT_GAMEPAD* Pad = &ControllerStatus.Gamepad;
						
						bool Up 			= (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool Down 			= (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool Left 			= (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool Right 			= (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool Start 			= (Pad->wButtons & XINPUT_GAMEPAD_START);
						bool Back 			= (Pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool LeftShoulder 	= (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool RightShoulder 	= (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool AButton 		= (Pad->wButtons & XINPUT_GAMEPAD_A);
						bool BButton 		= (Pad->wButtons & XINPUT_GAMEPAD_B);
						bool XButton 		= (Pad->wButtons & XINPUT_GAMEPAD_X);
						bool YButton 		= (Pad->wButtons & XINPUT_GAMEPAD_Y);
						
						int16 StickX = Pad->sThumbLX;
						int16 StickY = Pad->sThumbLY;
					}
					else
					{
						//NOTE(guoliang): Controller is not available
					}
				}
				RenderWeirdGradient(GlobalBackbuffer, XOffset, YOffset);

				win32_window_dimension Dimension = Win32GetWindowDimension(Window);
				Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
						GlobalBackbuffer);


				++XOffset;
				YOffset += 2;
			}
			ReleaseDC(Window, DeviceContext);
        }
        else
        {
            //TODO(guoliang): Logging
        }
    }
    else
    {
        //TODO(guoliang): Logging
    }

	return 0;
}
