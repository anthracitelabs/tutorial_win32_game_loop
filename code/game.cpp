#include "game.h"

void RenderGame(game_state *GameState)
{
	// this should push render commands into a buffer, which will then be interpreted into draw calls by platform specific layer
}

void GameFrame(game_memory* Memory, game_input* Input)
{
	game_state *GameState = (game_state *)Memory->PermanentStorage;

	if(!Memory->IsInitialized)
	{
		// Initialize the arena using the rest of the permanent storage
		InitializeArena(&GameState->WorldArena, 
						Memory->PermanentStorageSize - sizeof(game_state),
						(uint8_t *)Memory->PermanentStorage + sizeof(game_state));

		Memory->IsInitialized = true;
	}
	
	// For a single object
	hero_entity *Hero = PushStruct(&GameState->WorldArena, hero_entity);
	Hero->Health = 100;
	Hero->x = 0.0f;
	Hero->y = 0.0f;
	
	// for multiple objects
	uint32_t MaxEnemies = 256;
	enemy_entity *Enemies = PushArray(&GameState->WorldArena, MaxEnemies, enemy_entity);

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
	RenderGame(GameState);
}


