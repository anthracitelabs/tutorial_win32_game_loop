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

void SDLProcessPendingMessages(game_controller_input *KeyboardController, v2* Mouse, SDL_Window* Window, bool& GlobalRunning)
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
			
			case SDL_EVENT_MOUSE_MOTION:
			{
				// event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel
				int winW, winH, pixW, pixH;
				SDL_GetWindowSize(Window, &winW, &winH);
				SDL_GetWindowSizeInPixels(Window, &pixW, &pixH);

				Mouse->x = (Event.motion.x * ((float)pixW / (float)winW)) / (float)pixW;
				Mouse->y = (Event.motion.y * ((float)pixH / (float)winH)) / (float)pixH;
				
				//SDL_Log("Motion: x=%.1f y=%.1f dx=%.1f dy=%.1f pixW=%.1f pixHy=%.1f winW=%.1f winH=%.1f", Event.motion.x, Event.motion.y, Event.motion.xrel, Event.motion.yrel, (float)pixW, (float)pixH, (float)winW, (float)winH);
				
			} break;
			
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				if (Event.button.button == SDL_BUTTON_LEFT) {
					// Click at (event.button.x, event.button.y), count: event.button.clicks
				}
			} break;
			
			case SDL_EVENT_MOUSE_WHEEL:
			{
				// event.wheel.x, event.wheel.y
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

void input(game_input* OldInput, game_input* NewInput, SDL_Window* Window, bool& GlobalRunning)
{
	// ------------------------------------
	// KEYBOARD & MOUSE INPUT PROCESSING SECTION
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
	SDLProcessPendingMessages(NewKeyboardController, &NewInput->Mouse, Window, GlobalRunning);
	// ------------------------------------
	// KEYBOARD & MOUSE INPUT PROCESSING SECTION
	// ------------------------------------
}

#endif
