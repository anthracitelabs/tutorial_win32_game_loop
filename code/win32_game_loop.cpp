#include <windows.h>

#include <stdio.h>
#include <chrono>

#include "game_platform.h"
#include "game.cpp"


static bool Running;
static int red;
static int green;
static int blue;

// just to be able to printf on the console
void InitializeConsole() {
    // 1. Allocate a new console for the calling process
    AllocConsole();

    // 2. Redirect standard output (stdout) to the new console
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    
    // 3. Clear the error state of the stream
    clearerr(stdout);
}


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
    printf("\racc = %.2f vel = %.2f x = %.2f y = %.2f", state.a, state.v, state.x, state.y);
}

void Win32ProcessPendingMessages(game_input* Input)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
                {
                    Running = false;
                } break;

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
                {
                    uint32_t VKCode = (uint32_t)Message.wParam;
                    bool IsDown = ((Message.lParam & (1 << 31)) == 0);
                    bool WasDown = ((Message.lParam & (1 << 30)) != 0);

                    if(IsDown != WasDown)
                    {
                        if (VKCode == VK_UP) { Input->Up = !(Input->Up); }
                        else if (VKCode == VK_DOWN) { Input->Down = !(Input->Down); }
                        else if (VKCode == VK_LEFT) { Input->Left = !(Input->Left); }
                        else if (VKCode == VK_RIGHT) { Input->Right = !(Input->Right); }
                        else if (VKCode == VK_ESCAPE) { Running = false; }

                        bool AltKeyWasDown = ((Message.lParam & (1 << 29)) != 0);
                        if((VKCode == VK_F4) && AltKeyWasDown) { Running = false; }
                    }
                } break;

            default:
                {    
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                } break;
        }
    }
}

// Forward declaration of the window procedure callback function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	InitializeConsole(); // just for printf to work
	
	// 1. Register the Window Class
    WNDCLASS wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WindowProc;       // Pointer to the event handler function
    wc.hInstance     = hInstance;        // Handle to the application instance
    wc.lpszClassName = "Sample Window";       // Unique name identifying this class
    RegisterClass(&wc);

    // 2. Create the Window Instance
    HWND Window = CreateWindowEx(
        0,                               // Optional window styles
        wc.lpszClassName,                      // Window class name
        "My First WinAPI Window",       // Window title text
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,             // Standard window style (minimize, maximize, borders)
        // Size and position (using default OS coordinates)
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );
	
	if(Window)
	{
		// Metrics tracking
		int totalTicks = 0;         // Complete deterministic step counter
		int totalFrames = 0;        // Rendering frame counter
		int fpsCounter = 0;         // Resets every second
		int tpsCounter = 0;         // Resets every second (Ticks Per Second)
		
		Running = true;
		
		float t = 0.0;
		float dt = 0.1;
			
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		float accumulator = 0.0;
		
		State previousState = {0};
		State currentState = {0};
		
		std::chrono::steady_clock::time_point lastFpsUpdateTime;  // Last time metrics were displayed

		game_input Input = {};
		game_memory Memory = {};
	
		while(Running)
		{
			Win32ProcessPendingMessages(&Input); // input() function
			
			std::chrono::steady_clock::time_point newTime = std::chrono::steady_clock::now();
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

				GameFrame(&Memory, &Input);
				red = Memory.red;
				green = Memory.green;
				blue = Memory.blue;

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
			InvalidateRect(Window, NULL, FALSE);
			render(state);
			
			totalFrames++;
			fpsCounter++;

			// 4. PERFORMANCE RUNTIME TRACKING
			// Output metrics every 1 second
			std::chrono::duration<float> outputTime = currentTime - lastFpsUpdateTime;
			if (outputTime.count() >= 1.0f) {
				printf(" -> FPS (Render Rate): %d | TPS (Simulation Rate): %d | Total Ticks: %d\r", fpsCounter, tpsCounter, totalTicks);
				
				// Reset per-second counters
				fpsCounter = 0;
				tpsCounter = 0;
				lastFpsUpdateTime = currentTime;
			}
		}
	}
	
    return 0;
}

// 5. The Window Procedure (handles events like clicking close, resizing, painting)
LRESULT CALLBACK WindowProc(HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
		case WM_DESTROY:
			Running = false;
			PostQuitMessage(0); // Signals the message loop to stop when the window closes
			return 0;

		case WM_CLOSE:
			{
				Running = false;
			} break;

		case WM_SIZE:
			{
			} break;

		case WM_PAINT: // if this does not exist, a bigger resized section will not be painted, otherwise no major or functional effect 
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(Window, &ps);

				// Define the rectangle of the client area that needs painting
				RECT rect;
				GetClientRect(Window, &rect);

				// Create a solid brush (e.g., white or black)
				HBRUSH hBrush = CreateSolidBrush(RGB(red, green, blue)); // White canvas

				// Fill the canvas area
				FillRect(hdc, &rect, hBrush);

				// Clean up the brush
				DeleteObject(hBrush);

				EndPaint(Window, &ps);
			} break;
    }
    // Pass any unhandled messages to default Windows behavior
    return DefWindowProc(Window, uMsg, wParam, lParam);
}
