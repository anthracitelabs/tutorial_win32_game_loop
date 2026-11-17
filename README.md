# tutorial_win32_game_loop

This is a demonstration of a basic game loop implementation in win32. VSYNC should be enabled (possibly with OpenGL) to match display refresh rate. Sleep is not recommended as it is unreliable.

Source is the famous Glenn Fiedler article on Gaffer on Games website.

[Gaffer on Games - Fix your timestep!](https://gafferongames.com/post/fix_your_timestep/)

In the early days of the project, handmade.cpp was directly included in win32_handmade.cpp via #include "handmade.cpp"

However, as the architecture evolved to support live code editing (hot-reloading), this structure changed


Phase 1: The Unity Build (Early Days)

Initially, Casey compiled the entire project as a single translation unit (a "Unity Build"). To achieve this, the platform layer textually pasted the game code right into itself:
• win32_handmade.cpp explicitly used #include "handmade.cpp".
• The compiler handled it as one giant file, allowing quick compilation without a complex build system.

Phase 2: DLL Separation for Hot-Reloading (Later Days)

When live loop editing and hot-reloading were introduced, win32_handmade.cpp could no longer directly include handmade.cpp. They had to be split into two separate compilation units so the game could be compiled into a separate dynamic library (handmade.dll):
• win32_handmade.cpp: Compiles into win32_handmade.exe. It dropped the direct inclusion of handmade.cpp. Instead, it includes handmade_platform.h to understand the shared structs and function pointers needed to communicate with the game.
• handmade.cpp: Compiles independently into handmade.dll. The Win32 platform layer unloads and reloads this DLL at runtime whenever it detects a fresh build
 
 
In Handmade Hero, Casey Muratori implements the platform API using macros that expand into both function signatures and function pointer types, organized inside a global platform struct.
The exact C/C++ literal structure used in handmade_platform.h to handle cross-platform file reading and memory freeing is structured as follows:


### The Macro Definition Pattern

To avoid duplicating the function signatures (once for the platform's actual implementation and once for the function pointer type definition), Casey uses this pattern:

~~~~ C
#define PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(char *Filename)
typedef PLATFORM_READ_ENTIRE_FILE(platform_read_entire_file);

#define PLATFORM_FREE_FILE_MEMORY(name) void name(debug_read_file_result *FileMemory)
typedef PLATFORM_FREE_FILE_MEMORY(platform_free_file_memory);

#define PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(char *Filename, uint32 MemorySize, void *Memory)
typedef PLATFORM_WRITE_ENTIRE_FILE(platform_write_entire_file);
~~~~

What this does: The typedef line automatically generates a valid function pointer type named platform_read_entire_file* that matches the function signature defined in the macro.


### The Data Contract (debug_read_file_result)

Before passing the file data, both layers must agree on what a "loaded file" looks like. It's just a raw size and a raw chunk of memory:

~~~~ C
struct debug_read_file_result
{
    uint32 ContentsSize;
    void *Contents;
};
~~~~


### The Functional Boundary Struct (game_memory)

All of these generated function pointer types are packaged cleanly inside a struct (or a global platform struct API in later versions of the codebase) that is passed directly into the game layer's entry point:

~~~~ C
struct game_memory
{
    uint64 PermanentStorageSize;
    void *PermanentStorage; // REQUIRED to be cleared to zero at startup

    uint64 TransientStorageSize;
    void *TransientStorage; // REQUIRED to be cleared to zero at startup

    // Here are the literal function pointers the game can use:
    platform_read_entire_file *DEBUGReadEntireFile;
    platform_free_file_memory *DEBUGFreeFileMemory;
    platform_write_entire_file *DEBUGWriteEntireFile;
};
~~~~

### In the Platform Layer (win32_handmade.cpp)

The Windows layer writes the actual operating system code matching the macro signature, then assigns the address of that function to the struct:

~~~~ C
// 1. Implement the actual Win32 function using the macro
internal PLATFORM_READ_ENTIRE_FILE(Win32ReadEntireFile)
{
    debug_read_file_result Result = {};
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        // ... Win32 specific code to read data into memory ...
    }
    return(Result);
}

// 2. Assign it to the memory struct before calling the game
game_memory Memory = {};
Memory.DEBUGReadEntireFile = Win32ReadEntireFile; 
Memory.DEBUGFreeFileMemory = Win32FreeFileMemory;
~~~~

### In the Game Layer (handmade.cpp)

When GameUpdateAndRender(game_memory *Memory, ...) is called, the game executes the pointer blindly. It has zero knowledge of CreateFileA or Windows handles:

~~~~ C
void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{
    // The game executes the function pointer directly out of the struct!
    debug_read_file_result BitmapFile = Memory->DEBUGReadEntireFile("player_sprite.bmp");
    
    if(BitmapFile.Contents)
    {
        // Parse the bitmap contents here...
        
        // Free it when done
        Memory->DEBUGFreeFileMemory(&BitmapFile);
    }
}
~~~~

## Here is a good article on input handling with this algorithm

(Handling input using independent rendering)[https://jakubtomsu.github.io/posts/input_in_fixed_timestep/]

## Another article on game loops time management

(Game loop Time management)[https://andreleite.com/posts/2025/game-loop/fixed-timestep-game-loop/]

# Input Handling Notes

EndedDown is carried over from the previous frame because operating systems use event-driven messaging for keyboards that only reports state changes, not continuous held states.
If a player holds a key down across multiple frames without releasing it, the OS will not send new "key down" messages on subsequent frames (or will only send erratic OS key-repeat messages). Carrying EndedDown forward preserves the button's held state until an explicit release event arrives.

## Key Takeaways for Polled APIs

• No Event Queue Needed: Unlike keyboards, you do not need to copy OldState->EndedDown into NewState->EndedDown before polling because NewState->EndedDown is immediately overwritten by IsDown.
• State Diffing: OldState->EndedDown is still required so the helper can compute HalfTransitionCount by checking whether the state changed since the last frame.
• Sub-Frame Limitation: Because polled APIs only sample hardware once per frame, a button pressed and released within the same frame cannot be detected (so HalfTransitionCount for polled buttons is always 0 or 1).

## HalfTransitionCount Usage

HalfTransitionCount is used to detect edge events (button presses and releases) and to catch fast mid-frame taps that would otherwise be lost during frame rate drops.

Core Uses in Game Logic

• Detecting "Just Pressed" (Rising Edge): A button was newly pressed this frame if EndedDown is true and HalfTransitionCount > 0. This triggers single-fire actions like jumping, shooting, or selecting a menu item without repeating every frame.
• Detecting "Just Released" (Falling Edge): A button was newly released this frame if EndedDown is false and HalfTransitionCount > 0. This handles variable jump heights, releasing a charged bow shot, or ending a sprint.
• Catching Sub-Frame Taps: If a player taps and releases a key within the same frame (causing HalfTransitionCount == 2 and EndedDown == false), the game still registers the press. In a standard boolean system (is_down and was_down), both values would read false and the input would be dropped.
• Counting Multi-Presses in Slow Frames: If a game hitches or runs at a low frame rate and a player mashes a button twice in one frame (HalfTransitionCount == 4), the game can process both presses instead of losing one.


# Complete List of Input Helper Functions

## 1. Edge & Sub-Frame Trigger Helpers

• WasPressed(State) (Rising Edge + Sub-Frame Taps): Returns true if the button was pressed at least once during the frame, even if the player released it before the frame ended.

~~~~ C
inline bool32 WasPressed(game_button_state State) {
    bool32 Result = ((State.HalfTransitionCount > 1) ||
                     ((State.HalfTransitionCount == 1) && (State.EndedDown)));
    return Result;
}
/*
 * HalfTransitionCount > 1: Guarantees at least one full press occurred during the frame, even if the button ended up (EndedDown == false).
 * HalfTransitionCount == 1 && State.EndedDown: Catches a standard press where the button started up and ended down.
 */
~~~~

• WasReleased(State) (Falling Edge + Sub-Frame Releases): Returns true if the button was released at least once during the frame, even if it was pressed and released within that same frame.

~~~~ C
inline bool32 WasReleased(game_button_state State) {
    bool32 Result = ((State.HalfTransitionCount > 1) ||
                     ((State.HalfTransitionCount == 1) && (!State.EndedDown)));
    return Result;
}
~~~~


## 2. Continuous State Helpers

• IsDown(State) (Currently Held): Returns true if the button is physically held down at the end of the frame. Use this for continuous actions like movement, holding up a shield, or firing an automatic weapon.

~~~~ C
inline bool32 IsDown(game_button_state State) {
    return State.EndedDown;
}
~~~~

• IsHeld(State) (Held Without Transition): Returns true if the button is down and did not change state this frame (i.e., it was already down from the previous frame). Useful if you need to separate the initial press frame from sustained holding.

~~~~ C
inline bool32 IsHeld(game_button_state State) {
    return (State.EndedDown && (State.HalfTransitionCount == 0));
}
~~~~


## 3. Multi-Press Counting Helper

• GetPressCount(State) (Exact Presses Per Frame): Calculates the exact number of down-transitions that occurred during a single frame. Useful during low frame rates or hitches when rapid button mashing should trigger multiple discrete actions.

~~~~ C
inline int GetPressCount(game_button_state State) {
    int Count = State.HalfTransitionCount / 2;
    if ((State.HalfTransitionCount % 2 == 1) && State.EndedDown) {
        Count += 1;
    }
    return Count;
}
~~~~



## Quick Reference Summary

Helper Function	Trigger Condition				Common Gameplay Use Case
WasPressed	Pressed this frame (including mid-frame taps).	Jumping, swinging a sword, menu selection.
WasReleased	Released this frame (including mid-frame taps).	Variable jump height cutoff, releasing a charged shot.
IsDown		Down at the end of the frame.			Walking, sprinting, holding a block.
IsHeld		Down at the end of the frame with 0 transitions.Charging an ability after the initial press frame.
GetPressCount	Total number of press edges in one frame.	Rapid-fire semi-auto weapons during frame drops.



## Why IsDown() is the Right Choice for Walking

• IsDown(State) (Use this): Returns true on the initial press frame and every subsequent frame the key remains held. This provides smooth, uninterrupted walking from the very first frame the player touches the key.
• IsHeld(State) (Avoid for walking): Returns false on the initial press frame because HalfTransitionCount is 1. Using this would introduce a one-frame input lag before the character starts moving.
• WasPressed(State) (Avoid for walking): Only fires for a single frame when the button goes down. It is meant for discrete actions like jumping or dashing.

## Sample Implementation

~~~~ C
struct v2 {
    float x;
    float y;
};

void UpdatePlayerMovement(Player* player, const game_controller_input* controller, float dt) {
    v2 move_dir = { 0.0f, 0.0f };

    if (controller->IsAnalog) {
        // 1. Analog stick movement (already normalized and deadzone-adjusted by platform layer)
        move_dir.x = controller->StickAverageX;
        move_dir.y = controller->StickAverageY;
    } else {
        // 2. Digital movement using IsDown()
        if (IsDown(controller->MoveUp))    { move_dir.y += 1.0f; }
        if (IsDown(controller->MoveDown))  { move_dir.y -= 1.0f; }
        if (IsDown(controller->MoveLeft))  { move_dir.x -= 1.0f; }
        if (IsDown(controller->MoveRight)) { move_dir.x += 1.0f; }
    }

    // 3. Normalize diagonal digital movement so walking diagonally isn't ~1.41x faster
    float length_sq = (move_dir.x * move_dir.x) + (move_dir.y * move_dir.y);
    if (length_sq > 1.0f) {
        float length = sqrtf(length_sq);
        move_dir.x /= length;
        move_dir.y /= length;
    }

    // 4. Apply speed and delta time to update position
    player->position.x += move_dir.x * player->walk_speed * dt;
    player->position.y += move_dir.y * player->walk_speed * dt;
}
~~~~

## Important improvement note

A multi-frame input buffer stores recent button presses for a short window of time (typically 4 to 8 frames, or ~65–130 milliseconds) so an action still executes if the player presses the button slightly before the character is allowed to perform it.
This makes controls feel responsive instead of punishing—for example, letting a player press Jump a few frames before landing on the ground, or pressing Attack right before a previous swing finishes.

Two common approaches are Countdown Timer Buffer and Ring Buffer (History array)

* Countdown Timer Buffer : you store a small float timer on your player entity for each buffered action. Using a time duration (in seconds) rather than a raw frame count keeps the buffer window consistent regardless of frame rate. Best for standard single action buffering (jump buffering, attack queuing, coyote time)

# Ring Buffer :If your game needs to detect sequences (such as double-taps, quarter-circle motions, or multi-button cancels), you can store the last N frames of game_controller_input in a fixed-size circular buffer. Best for Fighting games, complex multi-button combos, and deterministic replay systems.

