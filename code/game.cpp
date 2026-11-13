#include "game.h"

	
void InitGame(game_memory* Memory)
{	
	if(Memory->IsInitialized)
	{
		game_state *GameState = (game_state *)Memory->PermanentStorage;
		
		// For a single object
		Hero = PushStruct(&GameState->WorldArena, hero_entity);
		Hero->Health = 100;
		Hero->x = 0.0f;
		Hero->y = 0.0f;
		
		// for multiple objects
		Enemies = PushArray(&GameState->WorldArena, MaxEnemies, enemy_entity);
	}
}

void build_render_frame(CommandBuffer* cb) {
    cmd_buffer_reset(cb);

	// 1. Producer: Record commands
    cmd_buffer_reset(cb);

    CmdClear* clear = CMD_PUSH(cb, CMD_CLEAR, CmdClear);
    *clear = { 0.1f, 0.1f, 0.15f, 1.0f };

    // Draw a red rectangle in the center
    CmdDrawRectangle* rect1 = CMD_PUSH(cb, CMD_DRAW_RECTANGLE, CmdDrawRectangle);
    *rect1 = { -0.25f, -0.25f, 2.5f, 2.5f, 1.0f, 0.2f, 0.2f, 1.0f };

    // Draw a smaller green rectangle overlapping it
    CmdDrawRectangle* rect2 = CMD_PUSH(cb, CMD_DRAW_RECTANGLE, CmdDrawRectangle);
    *rect2 = { 3.0f, 0.0f, 3.4f, 3.4f, 0.2f, 1.0f, 0.3f, 1.0f };
	
	// Draw a smaller green rectangle overlapping it
    CmdDrawRectangle* rect3 = CMD_PUSH(cb, CMD_DRAW_RECTANGLE, CmdDrawRectangle);
    *rect3 = { 13.0f, 10.0f, 5.0f, 10.4f, 0.2f, 0.0f, 0.9f, 1.0f };
}

void GameFrame(CommandBuffer* cb, game_memory* Memory, game_input* Input)
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
				Hero->x -= 1;
			}

			if(Controller->MoveRight.EndedDown)
			{
				Hero->x += 1;
			}
			
			if(Controller->MoveLeft.EndedDown)
			{
				Hero->y -= 1;
			}

			if(Controller->MoveRight.EndedDown)
			{
				Hero->y += 1;
			}
		}

		if(Controller->Button_A.EndedDown)
		{
			// GameState->GreenOffset += 1;
		}
	}

	// write_sound_output_buffer();
	
	// write_render_buffer();
	//RenderGame(GameState);
	build_render_frame(cb);
}


