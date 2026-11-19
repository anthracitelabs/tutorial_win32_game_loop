#include "game_world.h"

	
void InitGame(game_memory* GameMemory, float dt)
{	
	game_state *GameState = (game_state *)GameMemory->PermanentStorage;
	
	GameState->dt = dt;
	
	// Initialize the arena using the rest of the permanent storage
	InitializeArena(&GameState->WorldArena, 
					GameMemory->PermanentStorageSize - sizeof(game_state),
					(uint8_t *)GameMemory->PermanentStorage + sizeof(game_state));
					
	// Initialize the arena using the rest of the temporary storage
	InitializeArena(&GameState->CommandBufferArena, 
					GameMemory->TransientStorageSize - (sizeof(game_state) + GameMemory->PermanentStorageSize),
					(uint8_t *)GameMemory->PermanentStorage + (sizeof(game_state) + GameMemory->PermanentStorageSize));

	// For a single object
	Hero = PushStruct(&GameState->WorldArena, hero_entity);
	Hero->Health = 100;
	Hero->pos.x = 0.0f;
	Hero->pos.y = 0.0f;
	Hero->pre_pos.x = 0.0f;
	Hero->pre_pos.y = 0.0f;
	Hero->velocity.x = 10.0f;
	Hero->velocity.y = 10.0f;
	
	// for multiple objects
	Enemies = PushArray(&GameState->WorldArena, MaxEnemies, enemy_entity);
	
	GameMemory->IsInitialized = true;	
}

void GameFrame(game_memory* Memory, game_input* Input)
{
	if(Memory->IsInitialized)
	{
		game_state *GameState = (game_state *)Memory->PermanentStorage;
		
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

		Hero->pre_pos = Hero->pos;
		
		Hero->pos.x += move_dir.x * Hero->velocity.x * GameState->dt;
		Hero->pos.y += move_dir.y * Hero->velocity.y * GameState->dt;
		
		// write_sound_output_buffer();
		
		// write_render_buffer();
		//RenderGame(GameState);
	}
}

void build_render_frame(game_memory* Memory, float alpha) 
{
	if(Memory->IsInitialized)
	{	
		game_state *GameState = (game_state *)Memory->PermanentStorage;
		
		cmd_buffer_reset(&GameState->CommandBufferArena);

		CmdClear* clear = CMD_PUSH(&GameState->CommandBufferArena, CMD_CLEAR, CmdClear);
		*clear = { 0.1f, 0.1f, 0.15f, 1.0f };
		
		float sx = 0.0f;
		float sy = 0.0f;
		float TileLength = 2.0f;
		for(int i = 0; i < GetRowTileCount(); i++)
		{
			for(int j = 0; j < GetColumnTileCount(); j++)
			{
				uint8 Tile = GetTile(i, j);
				if(Tile == 1)
				{	
					CmdDrawRectangle* rect = CMD_PUSH(&GameState->CommandBufferArena, CMD_DRAW_RECTANGLE, CmdDrawRectangle);
					*rect = { sx + j * TileLength, sy + i * TileLength, TileLength, TileLength, 0.0f, 1.0f, 1.0f, 1.0f };
				}
			}
		}

		// interpolate Hero position
		float x = Hero->pos.x * alpha + Hero->pre_pos.x * (1.0f - alpha);
		float y = Hero->pos.y * alpha + Hero->pre_pos.y * (1.0f - alpha);
	 
		CmdDrawRectangle* rect1 = CMD_PUSH(&GameState->CommandBufferArena, CMD_DRAW_RECTANGLE, CmdDrawRectangle);
		*rect1 = { x, y, 0.5f, 0.5f, 1.0f, 0.2f, 0.2f, 1.0f };
	}
}

// Calculates forces/accelerations acting on the state
// For example: gravity, wind resistance, player input, or springs
/*float calculate_acceleration(const State& state, float t) {
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
}*/
