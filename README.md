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
