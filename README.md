# tutorial_win32_game_loop

This is a demonstration of a basic game loop implementation in win32. VSYNC should be enabled (possibly with OpenGL) to match display refresh rate. Sleep is not recommended as it is unreliable.

Source is the famous Glenn Fiedler article on Gaffer on Games website.

[Gaffer on Games - Fix your timestep!](https://gafferongames.com/post/fix_your_timestep/)

In the early days of the project, handmade.cpp was directly included in win32_handmade.cpp via #include "handmade.cpp"

However, as the architecture evolved to support live code editing (hot-reloading), this structure changed


Phase 1: The Unity Build (Early Days)

Initially, Casey compiled the entire project as a single translation unit (a "Unity Build"). To achieve this, the platform layer textually pasted the game code right into itself:
• win32_handmade.cpp explicitly used #include "handmade.cpp".
• The compiler handled it as one giant file, allowing quick compilation without a complex build system.

Phase 2: DLL Separation for Hot-Reloading (Later Days)

When live loop editing and hot-reloading were introduced, win32_handmade.cpp could no longer directly include handmade.cpp. They had to be split into two separate compilation units so the game could be compiled into a separate dynamic library (handmade.dll):
• win32_handmade.cpp: Compiles into win32_handmade.exe. It dropped the direct inclusion of handmade.cpp. Instead, it includes handmade_platform.h to understand the shared structs and function pointers needed to communicate with the game.
• handmade.cpp: Compiles independently into handmade.dll. The Win32 platform layer unloads and reloads this DLL at runtime whenever it detects a fresh build
 
 
In Handmade Hero, Casey Muratori implements the platform API using macros that expand into both function signatures and function pointer types, organized inside a global platform struct.
The exact C/C++ literal structure used in handmade_platform.h to handle cross-platform file reading and memory freeing is structured as follows:


### The Macro Definition Pattern

To avoid duplicating the function signatures (once for the platform's actual implementation and once for the function pointer type definition), Casey uses this pattern:

~~~~ C
#define PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(char *Filename)
typedef PLATFORM_READ_ENTIRE_FILE(platform_read_entire_file);

#define PLATFORM_FREE_FILE_MEMORY(name) void name(debug_read_file_result *FileMemory)
typedef PLATFORM_FREE_FILE_MEMORY(platform_free_file_memory);

#define PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(char *Filename, uint32 MemorySize, void *Memory)
typedef PLATFORM_WRITE_ENTIRE_FILE(platform_write_entire_file);
~~~~

What this does: The typedef line automatically generates a valid function pointer type named platform_read_entire_file* that matches the function signature defined in the macro.


### The Data Contract (debug_read_file_result)

Before passing the file data, both layers must agree on what a "loaded file" looks like. It's just a raw size and a raw chunk of memory:

~~~~ C
struct debug_read_file_result
{
    uint32 ContentsSize;
    void *Contents;
};
~~~~


### The Functional Boundary Struct (game_memory)

All of these generated function pointer types are packaged cleanly inside a struct (or a global platform struct API in later versions of the codebase) that is passed directly into the game layer's entry point:

~~~~ C
struct game_memory
{
    uint64 PermanentStorageSize;
    void *PermanentStorage; // REQUIRED to be cleared to zero at startup

    uint64 TransientStorageSize;
    void *TransientStorage; // REQUIRED to be cleared to zero at startup

    // Here are the literal function pointers the game can use:
    platform_read_entire_file *DEBUGReadEntireFile;
    platform_free_file_memory *DEBUGFreeFileMemory;
    platform_write_entire_file *DEBUGWriteEntireFile;
};
~~~~

### In the Platform Layer (win32_handmade.cpp)

The Windows layer writes the actual operating system code matching the macro signature, then assigns the address of that function to the struct:

~~~~ C
// 1. Implement the actual Win32 function using the macro
internal PLATFORM_READ_ENTIRE_FILE(Win32ReadEntireFile)
{
    debug_read_file_result Result = {};
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        // ... Win32 specific code to read data into memory ...
    }
    return(Result);
}

// 2. Assign it to the memory struct before calling the game
game_memory Memory = {};
Memory.DEBUGReadEntireFile = Win32ReadEntireFile; 
Memory.DEBUGFreeFileMemory = Win32FreeFileMemory;
~~~~

### In the Game Layer (handmade.cpp)

When GameUpdateAndRender(game_memory *Memory, ...) is called, the game executes the pointer blindly. It has zero knowledge of CreateFileA or Windows handles:

~~~~ C
void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{
    // The game executes the function pointer directly out of the struct!
    debug_read_file_result BitmapFile = Memory->DEBUGReadEntireFile("player_sprite.bmp");
    
    if(BitmapFile.Contents)
    {
        // Parse the bitmap contents here...
        
        // Free it when done
        Memory->DEBUGFreeFileMemory(&BitmapFile);
    }
}
~~~~


## Input Handling

~~~~ C
----------------------------------------------------------------------------------
----------------------------------------------------------------------------------
// game.cpp

// inside frame producing function 
GameUpdateAndRender(game_input* Input)
{

	for(int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
	{
		game_controller_input *Controller = &Input->Controllers[ControllerIndex];
		if(Controller->IsAnalog)
		{
			// Analog movement tuning
			// GameState->BlueOffset += (int)(4.0f*Controller->StickAverageX)
			// GameState->ToneHz = 256 + (int)(128.0f*Controller->StickAverageY)
		}
		else
		{
			// Digital movement tuning
			if(Controller->MoveLeft.EndedDown)
			{
				// GameState->BlueOffset -= 1;
			}
			
			if(Controller->MoveRight.EndedDown)
			{
				// GameState->BlueOffset += 1;
			}
		}

		if(Controller->Button_A.EndedDown)
		{
			// GameState->GreenOffset += 1;
		}
	}

// write_sound_output_buffer();
// write_render_buffer();
}
----------------------------------------------------------------------------------
----------------------------------------------------------------------------------

// game.h

struct game_button_state	// CONTROLLER BUTTON STATES
{
	int32 HalfTransitionCount;
	bool EndedDown;
};

struct game_controller_input	// SINGLE GAME CONTROLLER
{
	bool IsAnalog; 

	// Analog Part
	float StickAverageX;
	float StickAverageY;
    
	union
	{
		game_button_state Buttons[10];
		struct	
		{
			// Digital Part
			game_button_state MoveUp;
			game_button_state MoveDown;
			game_button_state MoveLeft;
			game_button_state MoveRight;
			game_button_state Button_A;
			game_button_state Button_B;
			game_button_state Button_X;
			game_button_state Button_Y;
			game_button_state LeftShoulder;
			game_button_state RightShoulder;
		};
	};
};

struct game_input	// ALL CONTROLLERS STORED IN A STRUCT
{
	game_controller_input Controllers[5]; // 4 game controllers and a keyboard
};
----------------------------------------------------------------------------------
----------------------------------------------------------------------------------
// win32_game.cpp

#if !defined(HANDMADE_H)
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

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

int WinMain()
{
	//....
	game_input Input[2] = {};
	game_input* NewInput = &Input[0];
	game_input* OldInput = &Input[1];

	while(!quit)
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
		
		GameUpdateAndRender(&Input);
		
		// Assign Old Input to New
		game_input *Temp = NewInput;
			NewInput = OldInput;
			OldInput = Temp;
	}
}
----------------------------------------------------------------------------------
----------------------------------------------------------------------------------

~~~~
