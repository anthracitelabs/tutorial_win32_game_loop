# tutorial_win32_game_loop

This is a demonstration of a basic game loop implementation in win32. Important thing to note is a comparison to real life wall clock 
* FAILS for a granular sleep setting (set to minimum value of 1 ms).
* PASSES for CPU melting version, instead of granular sleep.

shall be implemented like below with VSYNC enabled (possibly with OpenGL). Sleep is not recommended as it is unreliable.

~~~~ C
#include <stdio.h>
#include <chrono>

struct State {
	float a;
	float v;
	float x, y;
	float frames;
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
    printf("\racceleration = %.4f velocity = %.4f x = %.4f y = %.4f\tavg frames in sec = %.4f", state.a, state.v, state.x, state.y, state.frames);
}

int main() {
	
	bool quit = false;
	float renderedFrames = 0;
	
	float t = 0.0;
	float dt = 0.1;
		
	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	float accumulator = 0.0;
	
	State previousState = {0};
	State currentState = {0};
	
	float framesRenderedInSecond = 0;
	
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
		}
		
		const float alpha = accumulator / dt;
		
		
		State state = {
						currentState.a,
						currentState.v,
						currentState.x * alpha + previousState.x * (1.0f - alpha), 
						currentState.y * alpha + previousState.y * (1.0f - alpha),
						framesRenderedInSecond
					};
		
		render(state);
		renderedFrames++;
		framesRenderedInSecond = (1.0f / (t / renderedFrames));
	}
	
    return 0;
}

~~~~
