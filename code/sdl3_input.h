#if !defined(SDL3_INPUT_H)
#define SDL3_INPUT_H

#include "game_platform.h"

static void SDLProcessKeyboardMessage(game_button_state *NewState, bool IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

void SDLProcessPendingMessages(game_controller_input *KeyboardController, bool& GlobalRunning)
{
    SDL_Event Event;
	while (SDL_PollEvent(&Event)) 
	{
		switch(Event.type)
		{
			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
			{
				if(!Event.key.repeat)
				{
					bool IsDown = (Event.type == SDL_EVENT_KEY_DOWN);
					SDL_Scancode KeyCode = Event.key.scancode;
					
					if(KeyCode == SDL_SCANCODE_W)
					{
						SDLProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
					}
					if(KeyCode == SDL_SCANCODE_A)
					{
						SDLProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
					}
					if(KeyCode == SDL_SCANCODE_S)
					{
						SDLProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
					}
					if(KeyCode == SDL_SCANCODE_D)
					{
						SDLProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
					}
					if(KeyCode == SDL_SCANCODE_Q)
					{
						SDLProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
					}
					if(KeyCode == SDL_SCANCODE_E)
					{
						SDLProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
					}
					if(KeyCode == SDL_SCANCODE_UP)
					{
						SDLProcessKeyboardMessage(&KeyboardController->Button_A, IsDown);
					}
					if(KeyCode == SDL_SCANCODE_DOWN)
					{
						SDLProcessKeyboardMessage(&KeyboardController->Button_B, IsDown);
					}
					if(KeyCode == SDL_SCANCODE_LEFT)
					{
						SDLProcessKeyboardMessage(&KeyboardController->Button_X, IsDown);
					}
					if(KeyCode == SDL_SCANCODE_RIGHT)
					{
						SDLProcessKeyboardMessage(&KeyboardController->Button_Y, IsDown);
					}
					if(KeyCode == SDL_SCANCODE_ESCAPE)
					{
						GlobalRunning = false;
					}
				}
			} break;
			
			case SDL_EVENT_WINDOW_RESIZED:
			{
				glViewport(0, 0, Event.window.data1, Event.window.data2);
			} break;
			
			case SDL_EVENT_QUIT:
			{
				GlobalRunning = false;
			} break;
		}
	}
}

void input(game_input* OldInput, game_input* NewInput, bool& GlobalRunning)
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
	SDLProcessPendingMessages(NewKeyboardController, GlobalRunning);
	// ------------------------------------
	// KEYBOARD INPUT PROCESSING SECTION
	// ------------------------------------
}

#endif
