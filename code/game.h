#if !defined(GAME_H)
#define GAME_H

#include "game_platform.h"

struct game_button_state	// CONTROLLER BUTTON STATES
{
	int32 HalfTransitionCount;
	bool EndedDown;
};

struct game_controller_input	// SINGLE GAME CONTROLLER
{
	bool IsAnalog; 
	bool IsConnected;

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

#endif
