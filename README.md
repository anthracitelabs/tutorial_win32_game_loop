# tutorial_win32_game_loop

This is a demonstration of a basic game loop implementation in win32. VSYNC should be enabled (possibly with OpenGL) to match display refresh rate. Sleep is not recommended as it is unreliable.

Source is the famous Glenn Fiedler article on Gaffer on Games website.

[Gaffer on Games - Fix your timestep!](https://gafferongames.com/post/fix_your_timestep/)


## Input Handling

~~~~ C
----------------------------------------------------------------------------------
----------------------------------------------------------------------------------
// game.cpp
// inside frame producing function 
GameUpdateAndRender(game_input* Input)
{
game_controller_input *Input0 = Input->Controllers[0];
if(Input0.IsAnalog)
{
	// Analog movement tuning
	
	// Input.StartX;	
	// Input.MinX;	
	// Input.MaxX;	
	// Input.EndX;	
	
	// Input.StartY;	
	// Input.MinY;	
	// Input.MaxY;	
	// Input.EndY;

	XOffset += (int)(4.0f * Input0.EndX);
	ToneHz = 256 + (int)(128.0f * (Input0.EndY));	
}
else
{
	// Digital movement tuning
}

if(Input0.DownButtonEndedDown)
{
	YOffset += 1;
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
    	float StartX;
    	float StartY;
    	float MinX;
    	float MinY;
    	float MaxX;
    	float MaxY;
    	float EndX;
    	float EndY;
    
	// Digital Part
    	game_button_state Button_A;
    	game_button_state Button_B;
    	game_button_state Button_X;
    	game_button_state Button_Y;
    	game_button_state LeftShoulder;
    	game_button_state RightShoulder;
};

struct game_input	// ALL CONTROLLERS STORED IN A STRUCT
{
	game_controller_input Controllers[4];
};

----------------------------------------------------------------------------------
----------------------------------------------------------------------------------
// win32_game.cpp
#if !defined(HANDMADE_H)
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

void Win32ProcessPendingMessages(game_controller_input *KeyboardController)
{
    MSG Message;
    while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
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
        u32 VKCode = (u32)Message.wParam;
        bool IsDown = ((Message.lParam & (1 << 31)) == 0);
        bool WasDown = ((Message.lParam & (1 << 30)) != 0);
        
        if(IsDown != WasDown)
        {
            if (VKCode == 'W')
            {
            }
            else if (VKCode == 'A')
            {
            }
            else if (VKCode == 'S')
            {
            }
            else if (VKCode == 'D')
            {
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
		Win32ProcessKeyboardMessage(&KeyboardController->Up, IsDown);
            }
            else if (VKCode == VK_DOWN)
            {
		Win32ProcessKeyboardMessage(&KeyboardController->Down, IsDown);
            }
            else if (VKCode == VK_LEFT)
            {
		Win32ProcessKeyboardMessage(&KeyboardController->Left, IsDown);
            }
            else if (VKCode == VK_RIGHT)
            {
		Win32ProcessKeyboardMessage(&KeyboardController->Right, IsDown);
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

void Win32ProcessKeyboardMessage(game_button_state *NewState, b32 IsDown)
{
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


//....
game_input Input[2] = {};
game_input* OldInput = &Input[0];
game_input* NewInput = &Input[1];

int MaxControllerCount = XUSER_MAX_COUNT; 
if(MaxControllerCount > ArrayCount(NewInput->Controllers))
{
  	MaxControllerCount = ArrayCount(NewInput->Controllers);
}

while(!quit)
{
	game_controller_input *KeyboardController = &NewInput->Controllers[0];
    	game_controller_input ZeroController = {};
    	*KeyboardController = ZeroController;

    	Win32ProcessPendingMessages(KeyboardController);

	for (DWORD ControllerIndex = 0;
        	ControllerIndex < MaxControllerCount;
        	++ControllerIndex)
	{
    		game_controller_input *OldController = &OldInput->Controllers[ControllerIndex];
    		game_controller_input *NewController = &NewInput->Controllers[ControllerIndex];                
    		// ... 
    		// Read the controller input
    		// ... 
		NewController->IsAnalog = true;
		NewController->StartX = OldController->EndX;
		NewController->StartY = OldController->EndY;

		float X = Win32ProcessXInputStickValue(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);		
		float Y = Win32ProcessXInputStickValue (Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		NewController->StartX = OldController->EndX;
		NewController->StartY = OldController->EndY;
		NewController->MinX = NewController->MaxX = NewController->EndX = X;
		NewController->MinY = NewController->MaxY = NewController->EndY = Y;
		NewController->IsAnalog = true;		


		bool Up            = Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
		bool Down          = Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
		bool Left          = Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
		bool Right         = Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

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

	// Handle Keyboard messages
	Win32ProcessPendingMessages();
	
	GameUpdateAndRender(&Input);
	
	// Assign Old Input to New
	game_input *Temp = NewInput;
    	NewInput = OldInput;
    	OldInput = Temp;
}

----------------------------------------------------------------------------------
----------------------------------------------------------------------------------

~~~~
