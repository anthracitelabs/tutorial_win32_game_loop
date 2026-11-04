#include <stdio.h>
#include <chrono>

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

int main() {
	// Metrics tracking
	int totalTicks = 0;         // Complete deterministic step counter
	int totalFrames = 0;        // Rendering frame counter

	int fpsCounter = 0;         // Resets every second
	int tpsCounter = 0;         // Resets every second (Ticks Per Second)
	
	bool quit = false;
	
	float t = 0.0;
	float dt = 0.1;
		
	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	float accumulator = 0.0;
	
	State previousState = {0};
	State currentState = {0};
	
	std::chrono::steady_clock::time_point lastFpsUpdateTime;  // Last time metrics were displayed

	while(!quit)
	{
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
	
    return 0;
}
