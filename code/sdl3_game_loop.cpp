#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <chrono>
#include <stdlib.h>

#include "game.cpp"
#include "sdl3_input.h"

#include "opengl_loader.h"
#include "opengl_test.cpp"

static bool GlobalRunning;


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
			640, 480,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
			);

	if (!Window) {
		printf("Window creation failed: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}
	
	//SDL_SetWindowFullscreen(Window, true);
	
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
	float dt = 0.033;

	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	float accumulator = 0.0;

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
	
	GameMemory.IsInitialized = true;
		
	InitGame(&GameMemory, dt);
	// ------------------------------------
	// GAME MEMORY ALLOCATION SECTION
	// ------------------------------------

	gl_renderer_init();

	// initialize Gamepad 

	game_input Input[2] = {};
	game_input* NewInput = &Input[0];
	game_input* OldInput = &Input[1];
	
	std::chrono::steady_clock::time_point lastFpsUpdateTime = std::chrono::steady_clock::now();  // Last time metrics were displayed

	while (GlobalRunning) 
	{
		std::chrono::steady_clock::time_point newTime = std::chrono::steady_clock::now();

		input(OldInput, NewInput, Window, GlobalRunning);

		std::chrono::duration<float> frameTime = newTime - currentTime;

		float elapsedTime = frameTime.count();

		if(elapsedTime > 0.25f)
			elapsedTime = 0.25f;

		currentTime = newTime;

		accumulator += elapsedTime;

		while(accumulator >= dt)
		{
			GameFrame(&GameMemory, Input);

			t += dt;
			accumulator -= dt;

			totalTicks++;
			tpsCounter++;
		}

		const float alpha = accumulator / dt;

		build_render_frame(&GameMemory, alpha);

		//printf("\racc = %.2f vel = %.2f x = %.2f y = %.2f", state.a, state.v, state.x, state.y);
		
		gl_renderer_execute(&GameMemory);
		
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
