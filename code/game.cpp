#include "game.h"

void GameFrame(game_memory* Memory, game_input* Input)
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
