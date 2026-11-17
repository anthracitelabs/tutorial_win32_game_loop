#include "game.h"

	
void InitGame(game_memory* Memory, float dt)
{	
	if(Memory->IsInitialized)
	{
		game_state *GameState = (game_state *)Memory->PermanentStorage;
		
		GameState->dt = dt;

		// For a single object
		Hero = PushStruct(&GameState->WorldArena, hero_entity);
		Hero->Health = 100;
		Hero->pos.x = 0.0f;
		Hero->pos.y = 0.0f;
		Hero->velocity.x = 10.0f;
		Hero->velocity.y = 10.0f;
		
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
    *rect1 = { Hero->pos.x, Hero->pos.y, 2.5f, 2.5f, 1.0f, 0.2f, 0.2f, 1.0f };

    // Draw a smaller green rectangle overlapping it
    CmdDrawRectangle* rect2 = CMD_PUSH(cb, CMD_DRAW_RECTANGLE, CmdDrawRectangle);
    *rect2 = { 3.0f, 0.0f, 3.4f, 3.4f, 0.2f, 1.0f, 0.3f, 1.0f };
	
	// Draw a smaller green rectangle overlapping it
    CmdDrawRectangle* rect3 = CMD_PUSH(cb, CMD_DRAW_RECTANGLE, CmdDrawRectangle);
    *rect3 = { 13.0f, 10.0f, 5.0f, 10.4f, 0.2f, 0.0f, 0.9f, 1.0f };
}

void GameFrame(CommandBuffer* cb, game_memory* Memory, game_input* Input, float dt)
{
	game_state *GameState = (game_state *)Memory->PermanentStorage;

	if(!Memory->IsInitialized)
	{
		// Initialize the arena using the rest of the permanent storage
		InitializeArena(&GameState->WorldArena, 
						Memory->PermanentStorageSize - sizeof(game_state),
						(uint8_t *)Memory->PermanentStorage + sizeof(game_state));

		Memory->IsInitialized = true;
		InitGame(Memory, dt);
	}
	
	v2 move_dir = { 0.0f, 0.0f };	
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
				move_dir.x = -1.0f;
			}

			if(Controller->MoveRight.EndedDown)
			{
				move_dir.x = 1.0f;
			}
			
			if(Controller->MoveUp.EndedDown)
			{
				move_dir.y = 1.0f;
			}

			if(Controller->MoveDown.EndedDown)
			{
				move_dir.y = -1.0f;
			}
		}

		if(Controller->Button_A.EndedDown)
		{
			// GameState->GreenOffset += 1;
		}
	}
	
	float length_sq = (move_dir.x * move_dir.x) + (move_dir.y * move_dir.y);
    	if (length_sq > 1.0f) {
        	float length = sqrtf(length_sq);
        	move_dir.x /= length;
        	move_dir.y /= length;
    	}
	
	Hero->pos.x += move_dir.x * Hero->velocity.x * GameState->dt;
	Hero->pos.y += move_dir.y * Hero->velocity.y * GameState->dt;
	
	// write_sound_output_buffer();
	
	// write_render_buffer();
	//RenderGame(GameState);
	build_render_frame(cb);
}


