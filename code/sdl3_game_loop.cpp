#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <chrono>
#include <stdlib.h>

#include "game_platform.h"
#include "opengl_loader.h"
#include "game.cpp"
#include "opengl_test.cpp"

static bool GlobalRunning;


struct State {
	float a;
	float v;
	float x, y;
};

// Calculates forces/accelerations acting on the state
// For example: gravity, wind resistance, player input, or springs
float calculate_acceleration(const State& state, float t) {
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
}

void render(const State& state)
{   
    // 5. Erase the old character before moving to the next frame
    //printf("\racc = %.2f vel = %.2f x = %.2f y = %.2f", state.a, state.v, state.x, state.y);
}

static void SDLProcessKeyboardMessage(game_button_state *NewState, bool IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

void SDLProcessPendingMessages(game_controller_input *KeyboardController)
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

int main(int argc, char* argv[]) 
{
	// 1. Initialize SDL3
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return 1;
	}

	// 2. Configure OpenGL 3.3 Core Profile BEFORE creating the window
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// 3. Create window with SDL_WINDOW_OPENGL
	SDL_Window* Window = SDL_CreateWindow(
			"SDL3 + OpenGL",
			800, 600,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
			);

	if (!Window) {
		printf("Window creation failed: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	// 4. Create and activate the OpenGL context
	SDL_GLContext gl_context = SDL_GL_CreateContext(Window);
	if (!gl_context) {
		printf("Context creation failed: %s\n", SDL_GetError());
		SDL_DestroyWindow(Window);
		SDL_Quit();
		return 1;
	}
	SDL_GL_MakeCurrent(Window, gl_context);

	load_gl_functions();

	// Enable VSync (optional)
	SDL_GL_SetSwapInterval(1);

	// Metrics tracking
	int totalTicks = 0;         // Complete deterministic step counter
	int totalFrames = 0;        // Rendering frame counter
	int fpsCounter = 0;         // Resets every second
	int tpsCounter = 0;         // Resets every second (Ticks Per Second)

	GlobalRunning = true;

	float t = 0.0;
	float dt = 0.1;

	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	float accumulator = 0.0;

	State previousState = {0};
	State currentState = {0};

	// ------------------------------------
	// GAME MEMORY ALLOCATION SECTION
	// ------------------------------------
	game_memory GameMemory = {};
	GameMemory.PermanentStorageSize = 64000;
	GameMemory.TransientStorageSize = 32000;

	// Calculate total size needed
	uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;

	// Request a specific base address in Debug mode for consistent pointer layouts
	void *BaseAddress = 0;

	// Allocate the absolute giant contiguous block of memory
	void *EntireStorageBlock = malloc(TotalSize);

	if(EntireStorageBlock)
	{
		// Sub-divide the single giant block into our memory struct pointers
		GameMemory.PermanentStorage = EntireStorageBlock;
		GameMemory.TransientStorage = ((uint8_t *)GameMemory.PermanentStorage + 
				GameMemory.PermanentStorageSize);
	}
	else
	{
		// Handle memory allocation failure
	}
	// ------------------------------------
	// GAME MEMORY ALLOCATION SECTION
	// ------------------------------------

	InitRenderingTest();

	// initialize Gamepad 

	game_input Input[2] = {};
	game_input* NewInput = &Input[0];
	game_input* OldInput = &Input[1];

	std::chrono::steady_clock::time_point lastFpsUpdateTime = std::chrono::steady_clock::now();  // Last time metrics were displayed

	while (GlobalRunning) 
	{
		std::chrono::steady_clock::time_point newTime = std::chrono::steady_clock::now();

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
		SDLProcessPendingMessages(NewKeyboardController);
		// ------------------------------------
		// KEYBOARD INPUT PROCESSING SECTION
		// ------------------------------------

		std::chrono::duration<float> frameTime = newTime - currentTime;

		float elapsedTime = frameTime.count();

		if(elapsedTime > 0.25f)
			elapsedTime = 0.25f;

		currentTime = newTime;

		accumulator += elapsedTime;

		while(accumulator >= dt)
		{
			previousState = currentState;
			integrate(currentState, t, dt);

			GameFrame(&GameMemory, Input);

			t += dt;
			accumulator -= dt;

			totalTicks++;
			tpsCounter++;
		}

		const float alpha = accumulator / dt;


		State state = {
			currentState.a,
			currentState.v,
			currentState.x * alpha + previousState.x * (1.0f - alpha), 
			currentState.y * alpha + previousState.y * (1.0f - alpha),
		};

		render(state);

		// OpenGL Rendering
		glClearColor(0.5f, 0.15f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderTestFrame(state, alpha);

		// Swap front and back buffers
		SDL_GL_SwapWindow(Window);

		totalFrames++;
		fpsCounter++;

		// Assign Old Input to New
		game_input *Temp = NewInput;
		NewInput = OldInput;
		OldInput = Temp;

		// 4. PERFORMANCE RUNTIME TRACKING
		// Output metrics every 1 second
		std::chrono::steady_clock::time_point frameEndTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> outputTime = frameEndTime - lastFpsUpdateTime;
		if (outputTime.count() >= 1.0f) {
			printf(" -> FPS (Render Rate): %d | TPS (Simulation Rate): %d | Total Ticks: %d\r", fpsCounter, tpsCounter, totalTicks);

			// Reset per-second counters
			fpsCounter = 0;
			tpsCounter = 0;
			lastFpsUpdateTime = frameEndTime;
		}
	}

	// 7. Cleanup
	SDL_GL_DestroyContext(gl_context);
	SDL_DestroyWindow(Window);
	SDL_Quit();
	return 0;
}
