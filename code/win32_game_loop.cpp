#include <windows.h>
#include <chrono>

#ifdef PRINTF_ENABLED
#include <stdio.h>
#endif

#include "game_platform.h"
#include "game.cpp"

#include <xinput.h>


static bool GlobalRunning;
static int red;
static int green;
static int blue;

// -------------------------------------------------------------------------
// GAMEPAD INITIALIZATION --------------------------------------------------
// -------------------------------------------------------------------------
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state); // (or x_input_get_state)

X_INPUT_GET_STATE(XInputGetStateStub) {
    return(ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

static void Win32LoadXInput() {
    // Try newer XInput 1.4 (Win 8/10/11), fallback to 1.3 (DirectX SDK redistributable), then 9.1.0
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary) {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    if(!XInputLibrary) {
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }

    if(XInputLibrary) {
        // Clear the stubs and wire up the real functions from the DLL
        XInputGetState_ = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        
        // Defensive check: If GetProcAddress failed for some reason, restore stubs
        if(!XInputGetState_) { XInputGetState_ = XInputGetStateStub; }
    } else {
        // Diagnostic or logging could go here; stubs remain active safely
    }
}
// -------------------------------------------------------------------------
// GAMEPAD INITIALIZATION --------------------------------------------------
// -------------------------------------------------------------------------

#ifdef PRINTF_ENABLED
// just to be able to printf on the console
void InitializeConsole() {
    // 1. Allocate a new console for the calling process
    AllocConsole();

    // 2. Redirect standard output (stdout) to the new console
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    
    // 3. Clear the error state of the stream
    clearerr(stdout);
}
#endif

struct State {
	float a;
	float v;
	float x, y;
};

// Calculates forces/accelerations acting on the state
// For example: gravity, wind resistance, player input, or springs
float calculate_acceleration(const State& state, float t) {
    const float gravity = -9.81f;
    const float drag_coefficient = 0.1f;
    
    // Total acceleration = gravity minus air resistance proportional to velocity
    return gravity - (drag_coefficient * state.v);
}

void integrate(State& state, float t, float dt)
{
	// 1. Calculate acceleration for the current state
    float acceleration = calculate_acceleration(state, t);
    
	state.a = acceleration;
	
    // 2. Update velocity first (Semi-Implicit step)
    state.v += acceleration * dt;
    
    // 3. Update position using the *new* velocity
	state.y += state.v * dt;
}

void render(const State& state)
{   
    // 5. Erase the old character before moving to the next frame
    //printf("\racc = %.2f vel = %.2f x = %.2f y = %.2f", state.a, state.v, state.x, state.y);
}


void Win32ProcessKeyboardMessage(game_button_state *NewState, bool IsDown)
{
	// Assert(NewState->EndedDown != IsDown);
    NewState->EndedDown = IsDown;
    ++NewState->HalfTransitionCount;
}

void Win32ProcessXInputDigitalButton(DWORD XInputButtonState, 
                                game_button_state *OldState, DWORD ButtonBit,
                                game_button_state *NewState)
{
    NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

void Win32ProcessPendingMessages(game_controller_input *KeyboardController)
{
    MSG Message;
    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
			case WM_QUIT:
			{
				GlobalRunning = false;
			} break;
				
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				// Handle keyboard messages here
				uint32_t VKCode = (uint32_t)Message.wParam;
				bool IsDown = ((Message.lParam & (1 << 31)) == 0);
				bool WasDown = ((Message.lParam & (1 << 30)) != 0);
				
				if(IsDown != WasDown)
				{
					if (VKCode == 'W')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
					}
					else if (VKCode == 'A')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
					}
					else if (VKCode == 'S')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
					}
					else if (VKCode == 'D')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
					}
					else if (VKCode == 'Q')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
					}
					else if (VKCode == 'E')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
					}
					else if (VKCode == VK_UP)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->Button_A, IsDown);
					}
					else if (VKCode == VK_DOWN)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->Button_B, IsDown);
					}
					else if (VKCode == VK_LEFT)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->Button_X, IsDown);
					}
					else if (VKCode == VK_RIGHT)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->Button_Y, IsDown);
					}
					else if (VKCode == VK_ESCAPE)
					{
						GlobalRunning = false;
					}
					else if (VKCode == VK_SPACE)
					{
					}
					
					b32 AltKeyWasDown = ((Message.lParam & (1 << 29)) != 0);
					if((VKCode == VK_F4) && AltKeyWasDown)
					{
						GlobalRunning = false;
					}
				}
			} break;
			default:
			{
					TranslateMessage(&Message);
					DispatchMessage(&Message);
			} break;
       }
    }
}

float Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold)
{
    float Result = 0;
    
    if(Value < -DeadZoneThreshold)
    {
        Result = (float)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
    }
    else if(Value > DeadZoneThreshold)
    {
        Result = (float)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));
    }
    
    return (Result);
}
// Forward declaration of the window procedure callback function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	#ifdef PRINTF_ENABLED
	InitializeConsole();
	#endif
	
	// 1. Register the Window Class
    WNDCLASS wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WindowProc;       // Pointer to the event handler function
    wc.hInstance     = hInstance;        // Handle to the application instance
    wc.lpszClassName = "Sample Window";       // Unique name identifying this class
    RegisterClass(&wc);

    // 2. Create the Window Instance
    HWND Window = CreateWindowEx(
        0,                               // Optional window styles
        wc.lpszClassName,                      // Window class name
        "My First WinAPI Window",       // Window title text
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,             // Standard window style (minimize, maximize, borders)
        // Size and position (using default OS coordinates)
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );
	
	if(Window)
	{
		// Metrics tracking
		int totalTicks = 0;         // Complete deterministic step counter
		int totalFrames = 0;        // Rendering frame counter
		int fpsCounter = 0;         // Resets every second
		int tpsCounter = 0;         // Resets every second (Ticks Per Second)
		
		GlobalRunning = true;
		
		float t = 0.0;
		float dt = 0.1;
			
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		float accumulator = 0.0;
		
		State previousState = {0};
		State currentState = {0};
		
		std::chrono::steady_clock::time_point lastFpsUpdateTime;  // Last time metrics were displayed

		
		// ------------------------------------
		// GAME MEMORY ALLOCATION SECTION
		// ------------------------------------
		game_memory GameMemory = {};
		GameMemory.PermanentStorageSize = 64000;
		GameMemory.TransientStorageSize = 32000;

		// Calculate total size needed
		uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;

		// Request a specific base address in Debug mode for consistent pointer layouts
		void *BaseAddress = 0;

		// Allocate the absolute giant contiguous block of memory
		void *EntireStorageBlock = VirtualAlloc(BaseAddress, TotalSize, 
                                        MEM_RESERVE|MEM_COMMIT, 
                                        PAGE_READWRITE);

		if(EntireStorageBlock)
		{
			// Sub-divide the single giant block into our memory struct pointers
			GameMemory.PermanentStorage = EntireStorageBlock;
			GameMemory.TransientStorage = ((uint8_t *)GameMemory.PermanentStorage + 
										   GameMemory.PermanentStorageSize);
			
			// Note: VirtualAlloc automatically zeros out memory upon initial commit!
		}
		else
		{
			// Handle memory allocation failure
		}
		// ------------------------------------
		// GAME MEMORY ALLOCATION SECTION
		// ------------------------------------
		
		
		// initialize Gamepad 
		Win32LoadXInput();
		
		game_input Input[2] = {};
		game_input* NewInput = &Input[0];
		game_input* OldInput = &Input[1];

		while(GlobalRunning)
		{

			// ------------------------------------
			// KEYBOARD INPUT PROCESSING SECTION
			// ------------------------------------
			game_controller_input *OldKeyboardController = &OldInput->Controllers[0];
			game_controller_input *NewKeyboardController = &NewInput->Controllers[0];
			game_controller_input ZeroController = {};
			*NewKeyboardController = ZeroController;
			NewKeyboardController->IsConnected = true;

			// carry only EndedDown from OldKeyboardController, reset rest
			for(int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ++ButtonIndex)
			{
				NewKeyboardController->Buttons[ButtonIndex].EndedDown = OldKeyboardController->Buttons[ButtonIndex].EndedDown;
			}
			Win32ProcessPendingMessages(NewKeyboardController);
			// ------------------------------------
			// KEYBOARD INPUT PROCESSING SECTION
			// ------------------------------------

			// ------------------------------------
			// GAMEPAD INPUT PROCESSING SECTION
			// ------------------------------------
			DWORD MaxControllerCount = XUSER_MAX_COUNT + 1; 
			if(MaxControllerCount > ArrayCount(NewInput->Controllers))
			{
				MaxControllerCount = ArrayCount(NewInput->Controllers);
			}
			// iterate Game Controllers
			for (DWORD ControllerIndex = 0;
					ControllerIndex < MaxControllerCount;
					++ControllerIndex)
			{
				DWORD OurControllerIndex = ControllerIndex + 1;
				game_controller_input *OldController = &OldInput->Controllers[OurControllerIndex];
				game_controller_input *NewController = &NewInput->Controllers[OurControllerIndex];

				XINPUT_STATE ControllerState;
				if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
				{
					NewController->IsConnected = true;

					XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

					NewController->IsAnalog = true;
					NewController->StickAverageX = Win32ProcessXInputStickValue(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
					NewController->StickAverageY = Win32ProcessXInputStickValue (Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

					if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
					{
						NewController->StickAverageY = 1.0f;
					}
					if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
					{
						NewController->StickAverageY = -1.0f;
					}
					if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
					{
						NewController->StickAverageX = -1.0f;
					}
					if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
					{
						NewController->StickAverageX = 1.0f;
					}

					float Threshold = 0.5f;
					Win32ProcessXInputDigitalButton(
							(NewController->StickAverageX < -Threshold) ? 1 : 0,
							&OldController->MoveLeft, 1,
							&NewController->MoveLeft);
					Win32ProcessXInputDigitalButton(
							(NewController->StickAverageX > Threshold) ? 1 : 0,
							&OldController->MoveRight, 1,
							&NewController->MoveRight);
					Win32ProcessXInputDigitalButton(
							(NewController->StickAverageY < -Threshold) ? 1 : 0,
							&OldController->MoveUp, 1,
							&NewController->MoveUp);
					Win32ProcessXInputDigitalButton(
							(NewController->StickAverageY > Threshold) ? 1 : 0,
							&OldController->MoveDown, 1,
							&NewController->MoveDown);

					// bool Start         = Pad->wButtons & XINPUT_GAMEPAD_START;
					// bool Back          = Pad->wButtons & XINPUT_GAMEPAD_BACK;
					Win32ProcessXInputDigitalButton(Pad->wButtons,
							&OldController->Button_A,XINPUT_GAMEPAD_A,
							&NewController->Button_A);
					Win32ProcessXInputDigitalButton(Pad->wButtons,
							&OldController->Button_B,XINPUT_GAMEPAD_B,
							&NewController->Button_B);
					Win32ProcessXInputDigitalButton(Pad->wButtons,
							&OldController->Button_X,XINPUT_GAMEPAD_X,
							&NewController->Button_X);
					Win32ProcessXInputDigitalButton(Pad->wButtons,
							&OldController->Button_Y,XINPUT_GAMEPAD_Y,
							&NewController->Button_Y);
					Win32ProcessXInputDigitalButton(Pad->wButtons,
							&OldController->LeftShoulder,XINPUT_GAMEPAD_LEFT_SHOULDER,
							&NewController->LeftShoulder);
					Win32ProcessXInputDigitalButton(Pad->wButtons,
							&OldController->RightShoulder,XINPUT_GAMEPAD_RIGHT_SHOULDER,
							&NewController->RightShoulder);
				}
				else
				{
					NewController->IsConnected = false;
				}
			}
			// ------------------------------------
			// GAMEPAD INPUT PROCESSING SECTION
			// ------------------------------------

			std::chrono::steady_clock::time_point newTime = std::chrono::steady_clock::now();
			std::chrono::duration<float> frameTime = newTime - currentTime;

			float elapsedTime = frameTime.count();

			if(elapsedTime > 0.25f)
				elapsedTime = 0.25f;

			currentTime = newTime;

			accumulator += elapsedTime;

			while(accumulator >= dt)
			{
				previousState = currentState;
				integrate(currentState, t, dt);

				GameFrame(&GameMemory, Input);
				
				t += dt;
				accumulator -= dt;

				totalTicks++;
				tpsCounter++;
			}

			const float alpha = accumulator / dt;


			State state = {
				currentState.a,
				currentState.v,
				currentState.x * alpha + previousState.x * (1.0f - alpha), 
				currentState.y * alpha + previousState.y * (1.0f - alpha),
			};
			InvalidateRect(Window, NULL, FALSE);
			render(state);

			totalFrames++;
			fpsCounter++;

			// 4. PERFORMANCE RUNTIME TRACKING
			// Output metrics every 1 second
			std::chrono::duration<float> outputTime = currentTime - lastFpsUpdateTime;
			if (outputTime.count() >= 1.0f) {
				#ifdef PRINTF_ENABLED
				printf(" -> FPS (Render Rate): %d | TPS (Simulation Rate): %d | Total Ticks: %d\r", fpsCounter, tpsCounter, totalTicks);
				#endif
				
				// Reset per-second counters
				fpsCounter = 0;
				tpsCounter = 0;
				lastFpsUpdateTime = currentTime;
			}

			// Assign Old Input to New
			game_input *Temp = NewInput;
			NewInput = OldInput;
			OldInput = Temp;
		}
	}
	
    return 0;
}

// 5. The Window Procedure (handles events like clicking close, resizing, painting)
LRESULT CALLBACK WindowProc(HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
		case WM_DESTROY:
			GlobalRunning = false;
			PostQuitMessage(0); // Signals the message loop to stop when the window closes
			return 0;

		case WM_CLOSE:
			{
				GlobalRunning = false;
			} break;

		case WM_SIZE:
			{
			} break;

		case WM_PAINT: // if this does not exist, a bigger resized section will not be painted, otherwise no major or functional effect 
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(Window, &ps);

				// Define the rectangle of the client area that needs painting
				RECT rect;
				GetClientRect(Window, &rect);

				// Create a solid brush (e.g., white or black)
				HBRUSH hBrush = CreateSolidBrush(RGB(red, green, blue)); // White canvas

				// Fill the canvas area
				FillRect(hdc, &rect, hBrush);

				// Clean up the brush
				DeleteObject(hBrush);

				EndPaint(Window, &ps);
			} break;
    }
    // Pass any unhandled messages to default Windows behavior
    return DefWindowProc(Window, uMsg, wParam, lParam);
}
